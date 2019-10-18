all:
	gcc -o game.exe main.c $(shell pkg-config --cflags --libs sdl2)
