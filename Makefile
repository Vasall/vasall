CC=gcc
OUT=./build/game
CFLAGS=-Wall
LDFLAGS=$(shell pkg-config --cflags --libs sdl2)

$(OUT): ./objects/main.o ./objects/xsdl.o ./objects/vector.o ./objects/vector.o
	$(CC) -o $(OUT) ./objects/main.o ./objects/xsdl.o ./objects/vector.o $(LDFLAGS) $(CFLAGS)

./objects/main.o: ./res/main.c
	$(CC) -c ./res/main.c -o ./objects/main.o $(LDFLAGS) $(CFLAGS)

./objects/xsdl.o: ./res/xsdl.c
	$(CC) -c ./res/xsdl.c -o ./objects/xsdl.o $(LDFLAGS) $(CFLAGS)

./objects/vector.o: ./res/vector.c
	$(CC) -c ./res/vector.c -o ./objects/vector.o $(LDFLAGS) $(CFLAGS)
