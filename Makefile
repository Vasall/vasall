CC=gcc
OUT=./build/vasall-client
CFLAGS=-Wall -ansi -g 
#-Wno-unused
LDFLAGS=$(shell pkg-config --cflags --libs sdl2 SDL2_ttf SDL2_image) -lm

$(OUT): ./obj/main.o ./obj/update.o ./obj/vector.o ./obj/gui.o ./obj/camera.o ./obj/world.o
	$(CC) -o $(OUT) ./obj/main.o ./obj/update.o ./obj/vector.o ./obj/gui.o ./obj/camera.o ./obj/world.o $(LDFLAGS) $(CFLAGS)

./obj/main.o: ./src/main.c
	$(CC) -c ./src/main.c -o ./obj/main.o $(LDFLAGS) $(CFLAGS)

./obj/update.o: ./src/update.c
	$(CC) -c ./src/update.c -o ./obj/update.o $(LDFLAGS) $(CFLAGS)

./obj/vector.o: ./src/vector.c
	$(CC) -c ./src/vector.c -o ./obj/vector.o $(LDFLAGS) $(CFLAGS)

./obj/gui.o: ./src/gui.c
	$(CC) -c ./src/gui.c -o ./obj/gui.o $(LDFLAGS) $(CFLAGS)

./obj/camera.o: ./src/camera.c
	$(CC) -c ./src/camera.c -o ./obj/camera.o $(LDFLAGS) $(CFLAGS)

./obj/world.o: ./src/world.c
	$(CC) -c ./src/world.c -o ./obj/world.o $(LDFLAGS) $(CFLAGS)
