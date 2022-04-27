CFLAGS = -O3 -lmpdclient -Wall

all:
	gcc -o mpdqueueapi main.c ./json.c $(CFLAGS)

clean:
	rm -f prog *.o
