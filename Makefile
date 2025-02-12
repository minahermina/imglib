CC = clang

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -Wno-pedantic -Wall -O3 -ggdb  $(CPPFLAGS)
LDFLAGS = -L. -limglib -Limglib

# Source files and output
SRCS = image.c
SHARED_LIB = libimglib.so

EXAMPLE_SRC = main.c
EXAMPLE_TARGET = main

all: $(SHARED_LIB)
lib: $(SHARED_LIB)

$(SHARED_LIB): image.c image.h
	@echo "-- Compiling shared library: $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared -fPIC -o $@ $<

example: lib
	$(CC) $(CFLAGS) $(EXAMPLE_SRC) $(LDFLAGS) -o $(EXAMPLE_TARGET)

clean:
	rm -f $(SHARED_LIB) $(EXAMPLE_TARGET)
