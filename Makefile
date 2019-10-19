CC=gcc
OUT=./build/game
CFLAGS=-Wall
LDFLAGS=$(shell pkg-config --cflags --libs sdl2)

$(OUT): ./objects/main.o ./objects/xsdl.o ./objects/vector.o
	$(CC) -o $(OUT) ./objects/main.o ./objects/xsdl.o ./objects/vector.o $(LDFLAGS) $(CFLAGS)

./objects/main.o: ./client/main.c
	$(CC) -c ./client/main.c -o ./objects/main.o $(LDFLAGS) $(CFLAGS)

./objects/xsdl.o: ./client/xsdl.c
	$(CC) -c ./client/xsdl.c -o ./objects/xsdl.o $(LDFLAGS) $(CFLAGS)

./objects/vector.o: ./shared/vector.c
	$(CC) -c ./shared/vector.c -o ./objects/vector.o $(LDFLAGS) $(CFLAGS)

#./objects/arrayutils.o: ./shared/arrayutils.c
#	$(CC) -c ./shared/arrayutils.c -o ./objects/arrayutils.o $(LDFLAGS) $(CFLAGS)
