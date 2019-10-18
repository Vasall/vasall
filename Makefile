prog: ./objects/main.o ./objects/xsdl.o ./objects/xmath.o
	gcc -o prog ./objects/main.o ./objects/xsdl.o $(shell pkg-config --cflags --libs sdl2)

./objects/main.o: main.c
	gcc -c main.c -o ./objects/main.o $(shell pkg-config --cflags --libs sdl2)

./objects/xsdl.o: ./incl/xsdl.c
	gcc -c ./incl/xsdl.c -o ./objects/xsdl.o $(shell pkg-config --cflags --libs sdl2)

./objects/xmath.o: ./incl/xmath.h
	gcc -c ./incl/xmath.c -o ./objects/xmath.o $(shell pkg-config --cflags --libs sdl2)
