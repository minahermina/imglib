CC = clang

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -Wno-pedantic -Wall -O3 -ggdb
LIB_DIR = ./build
LDFLAGS = -L$(LIB_DIR)/ -Wl,-rpath=$(LIB_DIR) -limglib

SRC = image.c
SHARED_LIB = $(LIB_DIR)/libimglib.so

EXAMPLE_SRC = main.c
EXAMPLE_TARGET = main

all: $(SHARED_LIB)
lib: $(SHARED_LIB)

$(SHARED_LIB): $(SRC) image.h
	@echo "-- Compiling shared library: $@"
	$(CC) $(CPPFLAGS) $(SRC) $(CFLAGS) -shared -fPIC -o $(SHARED_LIB)

example: lib
	@echo "\n-- Compiling $(EXAMPLE_SRC): $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) $(EXAMPLE_SRC) -o $(EXAMPLE_TARGET) $(LDFLAGS)

clean:
	rm -f $(SHARED_LIB) $(EXAMPLE_TARGET)
