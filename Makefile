CC=gcc
OUT=./build/game
FLAGS=-Wall

game: ./objects/main.o ./objects/xsdl.o ./objects/vector.o ./objects/vector.o
	$(CC) -o $(OUT) ./objects/main.o ./objects/xsdl.o ./objects/vector.o $(shell pkg-config --cflags --libs sdl2) -Wall

./objects/main.o: ./res/main.c
	$(CC) -c ./res/main.c -o ./objects/main.o $(shell pkg-config --cflags --libs sdl2) -Wall

./objects/xsdl.o: ./res/xsdl.c
	$(CC) -c ./res/xsdl.c -o ./objects/xsdl.o $(shell pkg-config --cflags --libs sdl2) -Wall

./objects/vector.o: ./res/vector.c
	$(CC) -c ./res/vector.c -o ./objects/vector.o $(shell pkg-config --cflags --libs sdl2) -Wall
