CFLAGS = -O3 -lmpdclient

all:
	gcc -o mpdqueueapi ./src/main.c src/json.c $(CFLAGS)

clean:
	rm -f prog *.o
