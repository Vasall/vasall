CC=gcc
OUT=./build/game
CFLAGS=-Wall -ansi
LDFLAGS=$(shell pkg-config --cflags --libs sdl2)

$(OUT): ./objects/main.o ./objects/vector.o
	$(CC) -o $(OUT) ./objects/main.o ./objects/vector.o $(LDFLAGS) $(CFLAGS)

./objects/main.o: ./src/main.c
	$(CC) -c ./src/main.c -o ./objects/main.o $(LDFLAGS) $(CFLAGS)

./objects/vector.o: ./src/vector.c
	$(CC) -c ./src/vector.c -o ./objects/vector.o $(LDFLAGS) $(CFLAGS)
