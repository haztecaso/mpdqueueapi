#include <mpd/client.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./json.h"

static int handle_error(struct mpd_connection *conn){
    assert(mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS);
    fprintf(stderr, "MPD ERROR: %s\n", mpd_connection_get_error_message(conn));
    mpd_connection_free(conn);
    return EXIT_FAILURE;
}

void response_next_no_errors(struct mpd_connection *conn){
    mpd_response_next(conn);
    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS || !mpd_response_finish(conn))
        exit(handle_error(conn));
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

void json_append_song_tag(JsonNode *object, const struct mpd_song *song, enum mpd_tag_type tag_type){
    JsonNode *value;
    const char *str;
    if((str = mpd_song_get_tag(song, tag_type, 0)) != NULL)
        value = json_mkstring(str);
    else
        value = json_mknull();
    json_append_member(object, mpd_tag_name(tag_type), value);
}

JsonNode * serialize_song(const struct mpd_song *song, bool get_tags){
    JsonNode *result = json_mkobject();
    // json_append_member(result, "uri", json_mkstring(mpd_song_get_uri(song)));
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

int main(void) {
    // Parameters
    const int N_SONGS = 7;

    // mpd connection setup
    struct mpd_connection *conn = mpd_connection_new(NULL, 0, 30000);
    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
        exit(handle_error(conn));

    // mpd command list: get mpd status
    mpd_command_list_begin(conn, true);
    mpd_send_status(conn);
    mpd_command_list_end(conn);

    JsonNode * data = json_mkobject();

    struct mpd_status * status = mpd_recv_status(conn);
    if (status == NULL) exit(handle_error(conn));

    const unsigned int queue_version    = mpd_status_get_queue_version(status);
    const unsigned int queue_length     = mpd_status_get_queue_length(status);
    const unsigned int song_pos         = mpd_status_get_song_pos(status);
    const unsigned int next_song_pos    = mpd_status_get_next_song_pos(status);
    mpd_status_free(status);

    json_append_member(data, "queue_version", json_mknumber((double) queue_version));
    json_append_member(data, "queue_length",  json_mknumber((double) queue_length));
    json_append_member(data, "song_pos",      json_mknumber((double) song_pos));
    json_append_member(data, "next_song_pos", json_mknumber((double) next_song_pos));

    response_next_no_errors(conn);

    const unsigned int end_pos = song_pos + N_SONGS;

    mpd_command_list_begin(conn, true);
    mpd_send_list_queue_range_meta(conn, song_pos, end_pos);
    mpd_command_list_end(conn);

    JsonNode *songs = json_mkarray();
    struct mpd_song *song;
    while ((song = mpd_recv_song(conn)) != NULL){
        json_append_element(songs, serialize_song(song, true));
        mpd_song_free(song);
    }

    response_next_no_errors(conn);
    json_append_member(data, "songs", songs);

    // mpd connection closing
    mpd_connection_free(conn);

    char *data_str = json_stringify(data, NULL);
    puts(data_str);
    free(data_str);
    return 0;
}
