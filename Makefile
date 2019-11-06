CC=gcc
OUT=./build/game
CFLAGS=-Wall -ansi
LDFLAGS=$(shell pkg-config --cflags --libs sdl2 SDL2_ttf SDL2_image) -lm

$(OUT): ./objects/main.o ./objects/vector.o
	$(CC) -o $(OUT) ./objects/main.o ./objects/vector.o $(LDFLAGS) $(CFLAGS)

./objects/main.o: ./src/main.c
	$(CC) -c ./src/main.c -o ./objects/main.o $(LDFLAGS) $(CFLAGS)

#./objects/gui.o: ./src/gui.c
#	$(CC) -c ./src/gui.c -o ./objects/gui.o $(LDFLAGS) $(CFLAGS)

./objects/vector.o: ./src/vector.c
	$(CC) -c ./src/vector.c -o ./objects/vector.o $(LDFLAGS) $(CFLAGS)
