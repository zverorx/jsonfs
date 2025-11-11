# Compiler to use (default: gcc)
CC ?= gcc

# Build mode: 'debug' or 'release' (default: release)
BUILD ?= release

# Installation directory (default: /usr/local/bin/)
PREFIX ?= /usr/local/bin/

# Name of the final executable
TARGET = jsonfs

# Source directory
SRCDIR = src

# Include directory
INCDIR = include

# Binary directory
BINDIR = bin

# Object directory
OBJDIR = obj

# List of source files
SOURCES = $(SRCDIR)/main.c 			 \
		  $(SRCDIR)/fuse_callbacks.c \
		  $(SRCDIR)/jsonfs.c 		 \
		  $(SRCDIR)/handlers.c

# List of objects files
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

# List of header files
HEADERS = $(INCDIR)/common.h	\
		  $(INCDIR)/jsonfs.h	\
		  $(INCDIR)/handlers.h 

# Compiler flags
CFLAGS = -std=gnu99

# Preprocessor flags
CPPFLAGS = -I$(INCDIR)

# Calling the pkg_config utility
PKG_CONFIG_CFLAGS := $(shell pkg-config --cflags fuse3 jansson)
PKG_CONFIG_LIBS := $(shell pkg-config --libs fuse3 jansson)

# Add debug or optimization flags based on build mode
ifeq ($(BUILD), debug)
	CFLAGS += -Wall -g
else
	ifeq ($(BUILD), release)
		CFLAGS += -O2
	endif
endif

# Default target: build the program
all: $(TARGET)

# Link object files into the final executable
$(TARGET):$(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $(BINDIR)/$@ $(PKG_CONFIG_LIBS) 

# Compile .c files into .o files
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	@mkdir -p $(OBJDIR)
	$(CC) -c $(CFLAGS) $(PKG_CONFIG_CFLAGS) $(CPPFLAGS) $< -o $@

# Remove object files
clean:
	rm -fr $(OBJDIR)/

# Remove all generated files (objects and executable)
distclean:
	rm -fr $(BINDIR)/ $(OBJDIR)/

#Install the executable to the target directory
install:
	install -d $(PREFIX)
	install -m 755 ./$(BINDIR)/$(TARGET) $(PREFIX)

# Remove the installed executable
uninstall:
	rm $(PREFIX)/$(TARGET)

# Show help message
help:
	@echo "Hints:"
	@echo ""
	@echo "make - Build the program"
	@echo "make BUILD=debug - Build with debug flags"
	@echo "make BUILD=release - Build with optimization"
	@echo ""
	@echo "make clean - Remove all temporary files"
	@echo "make distclean - Remove all generated files"
	@echo "make install - Install the executable file to $(PREFIX)"
	@echo "make uninstall - Remove the installed executable file from $(PREFIX)"
	@echo ""
	@echo "To change the installation and removal path, use PREFIX="

.PHONY: all clean distclean help install uninstall
