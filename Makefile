# ------------------------------------------------
# Generic Makefile, capable of including static
# libraries
#
# Modified by:      admin@enudstudios.com
# Date:             2020-01-09
#
# Original Author:  yanick.rochon@gmail.com
# Date:             2011-08-10
# ------------------------------------------------

# Name of the created executable
TARGET     := vasall-client

# Get the absolute path to the directory this makefile is in
MKFILE_PTH := $(abspath $(lastword $(MAKEFILE_LIST)))
MKFILE_DIR := $(dir $(MKFILE_PTH))

# All subdirectories in the lib-folder
LIB_PTH    := lib
LIB_DIRS   := $(sort $(dir $(wildcard $(MKFILE_DIR)$(LIB_PTH)/*/)))

# Set static libararies
LIBS       := 

# The compiler to use
CC         := gcc
# Error flags for compiling
ERRFLAGS   := -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition
# Compiling flags here
CFLAGS     := -g -O0 -ansi -std=c89 -pedantic -I. -I./include/ -I./$(LIB_PTH)/
SDL_CFLAGS := $(shell pkg-config --cflags sdl2 SDL2_ttf SDL2_image)
override CFLAGS += $(SDL_CFLAGS)


# The linker to use
LINKER     := gcc
# Linking flags here
LFLAGS     := -Wall -I. $(LIBS) -lm -lGL -lGLU -lglut
SDL_LFLAGS  := $(shell pkg-config --libs sdl2 SDL2_ttf SDL2_image)
override LFLAGS += $(SDL_LFLAGS)

# Change these to proper directories where each file should be
SRCDIR     := src
OBJDIR     := obj
BINDIR     := bin

SOURCES    := $(wildcard $(SRCDIR)/*.c)
INCLUDES   := $(wildcard $(SRCDIR)/*.h)
OBJECTS    := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

rm         := rm -f

$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $(OBJECTS) $(LFLAGS) -o $@
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) $(ERRFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

# Build all submodules in the lib-folder
.PHONY: libs
libs: ${LIB_DIRS}
	$(foreach dir,$(LIB_DIRS),make -C $(dir);)

