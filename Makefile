# CC=g++
CC_FLAGS=--std=c++17
LIB_FLAGS=-lmpdclient

all:
	$(CC) $(CC_FLAGS) $(NIX_CFLAGS_COMPILE) -o mpdqueueapi src/main.cpp src/json.cpp $(LIB_FLAGS)

clean:
	rm -f mpdqueueapi *.o
