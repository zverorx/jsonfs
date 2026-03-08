CC ?= gcc

# Build mode: 'debug' or 'release'
BUILD ?= release

# Installation directory
PREFIX ?= /usr/local/bin/

SRCDIR = src
INCDIR = include
BINDIR = bin
OBJDIR = obj

TARGET = $(BINDIR)/jsonfs

SOURCES = $(SRCDIR)/main.c				\
		  $(SRCDIR)/fuse_callbacks.c	\
		  $(SRCDIR)/handlers.c			\
		  $(SRCDIR)/json_operations.c	\
		  $(SRCDIR)/jsonfs.c			\
		  $(SRCDIR)/file_time.c

OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

HEADERS = $(INCDIR)/common.h			\
		  $(INCDIR)/handlers.h			\
		  $(INCDIR)/json_operations.h	\
		  $(INCDIR)/jsonfs.h			\
		  $(INCDIR)/file_time.h

CFLAGS = -std=gnu99
CPPFLAGS = -I$(INCDIR)
PKG_CONFIG_CFLAGS := $(shell pkg-config --cflags fuse3 jansson)
PKG_CONFIG_LIBS := $(shell pkg-config --libs fuse3 jansson)

# Add debug or optimization flags based on build mode
ifeq ($(BUILD), debug)
	CFLAGS += -Wall -g
else
	ifeq ($(BUILD), release)
		CFLAGS += -O2 -s
	endif
endif

all: $(TARGET)

$(TARGET):$(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $@ $(PKG_CONFIG_LIBS) 

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	@mkdir -p $(OBJDIR)
	$(CC) -c $(CFLAGS) $(PKG_CONFIG_CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -fr $(OBJDIR)/

distclean:
	rm -fr $(BINDIR)/ $(OBJDIR)/

install:
	install -d $(PREFIX)
	install -m 755 $(TARGET) $(PREFIX)

uninstall:
	rm $(PREFIX)/$(TARGET)

help:
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
