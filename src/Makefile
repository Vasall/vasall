CC            := gcc
OUT_FILE_NAME := xsdl.a

CFLAGS        := -fPIC -O0 -g -Wall -c -pedantic -std=c89
SDL_FLAGS     := $(shell pkg-config --cflags --libs sdl2 SDL2_ttf SDL2_image)
override CFLAGS += $(SDL_FLAGS)

# The directory to put the object-files into
OBJ_DIR       := ./obj

# The directory to put the final lib-file into
OUT_DIR       := ./lib

$(OUT_FILE_NAME): $(patsubst %.c,$(OBJ_DIR)/%.o,$(wildcard *.c))
	@ar -r -o $(OUT_DIR)/$@ $^
	@echo "Linking complete!"

$(OBJ_DIR)/%.o: %.c dirmake
	@$(CC) -c $(CFLAGS) -o $@  $<
	@echo "Compiled "$<" successfully!"

dirmake:
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(OBJ_DIR)
