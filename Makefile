CC=gcc
OUT=game

game: ./objects/main.o ./objects/vector.o ./objects/xmath.o
	$(CC) -o $(OUT) ./objects/main.o ./objects/xsdl.o ./objects/vector.o $(shell pkg-config --cflags --libs sdl2)

./objects/main.o: main.c
	$(CC) -c main.c -o ./objects/main.o $(shell pkg-config --cflags --libs sdl2)

./objects/xsdl.o: ./incl/xsdl.c
	$(CC) -c ./incl/xsdl.c -o ./objects/xsdl.o $(shell pkg-config --cflags --libs sdl2)

./objects/vector.o: ./incl/vector.c
	$(CC) -c ./incl/vector.c -o ./objects/vector.o $(shell pkg-config --cflags --libs sdl2)
