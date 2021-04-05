# ------------------------------------------------
# Generic Makefile, capable of including static
# libraries
#
# Modified by:      admin@vasall.net
# Date:             2021-04-06
#
# Original Author:  yanick.rochon@gmail.com
# Date:             2011-08-10
# ------------------------------------------------

# Name of the created executable
TARGET     := vasall

# Get the absolute path to the directory this makefile is in
MKFILE_PTH := $(abspath $(lastword $(MAKEFILE_LIST)))
MKFILE_DIR := $(dir $(MKFILE_PTH))

# All subdirectories in the lib-folder
LIB_PTH    := lib
LIB_DIRS   := $(sort $(dir $(wildcard $(MKFILE_DIR)$(LIB_PTH)/*/)))

# The compiler to use
CC         := gcc
# Error flags for compiling
ERRFLAGS   := -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition
# Compiling flags here
CFLAGS     := -g -O0 -ansi -std=c89 -pedantic -I. -I./inc/ -I./$(LIB_PTH)/
SDL_CFLAGS := $(shell pkg-config --cflags sdl2 SDL2_ttf SDL2_image)
override CFLAGS += $(SDL_CFLAGS)

# The linker to use
LINKER     := gcc
# Set libararies (FIXME: switch the variables)
LIBS       := $(shell find $(MKFILE_DIR)$(LIB_PTH)/ -type f -name "*.a")
STAT_LIBS  := -lm -lgmp -lGL -lcrypto -lvulkan
override LIBS += $(STAT_LIBS)
# Linking flags here
LFLAGS     := -Wall -I. $(LIBS)
SDL_LFLAGS  := $(shell pkg-config --libs sdl2 SDL2_ttf SDL2_image)
override LFLAGS += $(SDL_LFLAGS)

# Change these to proper directories where each file should be
SRCDIR     := src
OBJDIR     := obj
BINDIR     := bin
RESDIR     := res

SOURCES    := $(wildcard $(SRCDIR)/*.c)
INCLUDES   := $(wildcard $(SRCDIR)/*.h)
OBJECTS    := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
VSHADERS   := $(wildcard $(RESDIR)/shaders/*.vert)
FSHADERS   := $(wildcard $(RESDIR)/shaders/*.frag)
SVSHADERS  := $(VSHADERS:%.vert=%.vert.spv)
SFSHADERS  := $(FSHADERS:%.frag=%.frag.spv)

rm         := rm -f

$(BINDIR)/$(TARGET): $(OBJECTS) $(SVSHADERS) $(SFSHADERS)
	@$(LINKER) $(OBJECTS) $(LFLAGS) -o $@
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) $(ERRFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

%.vert.spv: %.vert
	@glslangValidator --target-env vulkan1.1 -o $@ $<
	@echo "Compiled "$<" successfully!"

%.frag.spv: %.frag
	@glslangValidator --target-env vulkan1.1 -o $@ $<
	@echo "Compiled "$<" successfully!"

# Create the directories to store the object-files and the final binary
.PHONY: dirs
dirs:
	mkdir -p obj
	mkdir -p bin

# Build all submodules in the lib-folder
.PHONY: libs
libs: ${LIB_DIRS}
	$(foreach dir,$(LIB_DIRS),make -C $(dir);)

