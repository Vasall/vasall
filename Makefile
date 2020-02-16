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
LIBS       :=  ./$(LIB_PTH)/XSDL/lib/xsdl.a

CC         := gcc
# Error flags for compiling
ERRFLAGS   := -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition
# Compiling flags here
CFLAGS     := -g -O0 -ansi -std=c89 -pedantic -I. -I./$(LIB_PTH)/  \
	$(shell pkg-config --cflags --libs sdl2 SDL2_ttf SDL2_image)

LINKER     := gcc
# Linking flags here
LFLAGS     := -Wall -I. $(LIBS) -lm -lGL -lGLU -lglut \
	$(shell pkg-config --cflags --libs sdl2 SDL2_ttf SDL2_image)

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

subsdirs: $(LIB_DIRS)
	$(MAKE) --directory=$@

.PHONY: clean
clean:
	@$(rm) $(OBJECTS)
	@echo "Cleanup complete!"

.PHONY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"
