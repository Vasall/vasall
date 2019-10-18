CC=gcc
OUT=game

game: ./objects/main.o ./objects/xsdl.o ./objects/xmath.o
	$(CC) -o $(OUT) ./objects/main.o ./objects/xsdl.o ./objects/xmath.o $(shell pkg-config --cflags --libs sdl2)

./objects/main.o: main.c
	$(CC) -c main.c -o ./objects/main.o $(shell pkg-config --cflags --libs sdl2)

./objects/xsdl.o: ./incl/xsdl.c
	$(CC) -c ./incl/xsdl.c -o ./objects/xsdl.o $(shell pkg-config --cflags --libs sdl2)

./objects/xmath.o: ./incl/xmath.c
	$(CC) -c ./incl/xmath.c -o ./objects/xmath.o $(shell pkg-config --cflags --libs sdl2)
