#include <mpd/client.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./json.h"

#define MAXLEN 80
#define EXTRA 5
#define MAXINPUT MAXLEN+EXTRA+2

static int handle_error(struct mpd_connection *c){
    assert(mpd_connection_get_error(c) != MPD_ERROR_SUCCESS);

    fprintf(stderr, "MPD ERROR: %s\n", mpd_connection_get_error_message(c));
    mpd_connection_free(c);
    return EXIT_FAILURE;
}

const char * mpd_tag_name(enum mpd_tag_type tag_type){
    switch (tag_type) {
        case MPD_TAG_ARTIST: return "artist";
        case MPD_TAG_ALBUM: return "album";
        case MPD_TAG_ALBUM_ARTIST: return "album_artist";
        case MPD_TAG_TITLE: return "title";
        case MPD_TAG_TRACK: return "track";
        case MPD_TAG_NAME: return "name";
        case MPD_TAG_GENRE: return "genre";
        case MPD_TAG_DATE: return "date";
        case MPD_TAG_ORIGINAL_DATE: return "original_date";
        case MPD_TAG_COMPOSER: return "composer";
        case MPD_TAG_PERFORMER: return "performer";
        case MPD_TAG_COMMENT: return "comment";
        case MPD_TAG_DISC: return "disc";
        case MPD_TAG_LABEL: return "label";

        case MPD_TAG_MUSICBRAINZ_ARTISTID: return "musicbrainz_artistid";
        case MPD_TAG_MUSICBRAINZ_ALBUMID: return "musicbrainz_albumid";
        case MPD_TAG_MUSICBRAINZ_ALBUMARTISTID: return "musicbrainz_albumartistid";
        case MPD_TAG_MUSICBRAINZ_TRACKID: return "musicbrainz_trackid";
        case MPD_TAG_MUSICBRAINZ_RELEASETRACKID: return "musicbrainz_releasetrackid";
        case MPD_TAG_MUSICBRAINZ_WORKID: return "musicbrainz_workid";

        default: return "";
    }
}

JsonNode * serialize_tag(const struct mpd_song *song, enum mpd_tag_type type){
    const char *value;
    if((value = mpd_song_get_tag(song, type, 0)) != NULL)
        return json_mkstring(value);
    else
        return json_mknull();
}

void json_append_song_tag(JsonNode *object, const struct mpd_song *song, enum mpd_tag_type tag_type){
    json_append_member(object, mpd_tag_name(tag_type), serialize_tag(song, tag_type));
}

JsonNode * serialize_song(const struct mpd_song *song, bool get_tags){
    JsonNode *result = json_mkobject();
    json_append_member(result, "id", json_mknumber((double) mpd_song_get_id(song)));
    json_append_member(result, "uri", json_mkstring(mpd_song_get_uri(song)));
    json_append_member(result, "duration", json_mknumber((double) mpd_song_get_duration(song)));
    if(get_tags){
        JsonNode *tags = json_mkobject();

        json_append_song_tag(tags, song, MPD_TAG_TITLE);
        json_append_song_tag(tags, song, MPD_TAG_ARTIST);
        json_append_song_tag(tags, song, MPD_TAG_ALBUM);
        json_append_song_tag(tags, song, MPD_TAG_ALBUM_ARTIST);
        json_append_song_tag(tags, song, MPD_TAG_GENRE);
        json_append_song_tag(tags, song, MPD_TAG_DATE);
        json_append_song_tag(tags, song, MPD_TAG_ORIGINAL_DATE);
        
        json_append_song_tag(tags, song, MPD_TAG_MUSICBRAINZ_ARTISTID);
        json_append_song_tag(tags, song, MPD_TAG_MUSICBRAINZ_ALBUMID);
        json_append_song_tag(tags, song, MPD_TAG_MUSICBRAINZ_TRACKID);

        json_append_member(result, "tags", tags);
    }
    return result;
}

JsonNode * get_queue_info(struct mpd_connection *conn){
    mpd_command_list_begin(conn, true);
    mpd_send_status(conn);
    mpd_command_list_end(conn);

    struct mpd_status * status = mpd_recv_status(conn);
    if (status == NULL) exit(handle_error(conn));

    JsonNode *result = json_mkobject();
    json_append_member(result, "version", json_mknumber((double) mpd_status_get_queue_version(status)));
    json_append_member(result, "length", json_mknumber((double) mpd_status_get_queue_length(status)));
    json_append_member(result, "position", json_mknumber((double) mpd_status_get_song_pos(status)));
    json_append_member(result, "current_id", json_mknumber((double) mpd_status_get_song_id(status)));
    json_append_member(result, "next_id", json_mknumber((double) mpd_status_get_next_song_id(status)));
    mpd_status_free(status);

    mpd_response_next(conn);
    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS || !mpd_response_finish(conn))
        exit(handle_error(conn));
    return result;
}

JsonNode * get_queue_songs(struct mpd_connection *conn, bool get_tags){
    mpd_command_list_begin(conn, true);
    mpd_send_list_queue_meta(conn);
    mpd_command_list_end(conn);

    JsonNode *result = json_mkarray();
    struct mpd_song *song;
    while ((song = mpd_recv_song(conn)) != NULL){
        json_append_element(result, serialize_song(song, get_tags));
        mpd_song_free(song);
    }

    mpd_response_next(conn);
    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS || !mpd_response_finish(conn))
        exit(handle_error(conn));
    return result;
}

JsonNode * get_queue_data(bool get_songs, bool get_songs_tags){
    struct mpd_connection *conn;
    conn = mpd_connection_new(NULL, 0, 30000);
    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
        exit(handle_error(conn));

    JsonNode *result = get_queue_info(conn);
    if(get_songs){
        JsonNode *songs = get_queue_songs(conn, get_songs_tags);
        json_append_member(result, "songs", songs);
    }
    mpd_connection_free(conn);
    return result;
}

void unencode(char *src, char *last, char *dest)
{
    for(; src != last; src++, dest++)
        if(*src == '+')
            *dest = ' ';
        else if(*src == '%') {
            int code;
            if(sscanf(src+1, "%2x", &code) != 1) code = '?';
            *dest = code;
            src +=2; }     
        else
            *dest = *src;
    *dest = '\n';
    *++dest = '\0';
}


int main_cgi(int argc, char ** argv) {
    char *lenstr;
    char input[MAXINPUT], data[MAXINPUT];
    long len;
    lenstr = getenv("CONTENT_LENGTH");

    if(lenstr == NULL || sscanf(lenstr,"%ld",&len)!=1 || len > MAXLEN){
        printf("Content-Type:text/plain;charset=utf-8\n");
        printf("Status: 400 Bad Request\n\n");
    }
    else {
        fgets(input, len+1, stdin);
        unencode(input+EXTRA, input+len, data);
        printf("Content-Type: application/json;charset=utf-8\n\n");
        JsonNode * queue_data = get_queue_data(true, true);
        char *tmp = json_stringify(queue_data, "  ");
        puts(tmp);
        free(tmp);
    }

    return 0;
}

int main(int argc, char ** argv) {
    JsonNode * queue_data = get_queue_data(true, true);
    char *tmp = json_stringify(queue_data, "  ");
    puts(tmp);
    free(tmp);
    return 0;
}
