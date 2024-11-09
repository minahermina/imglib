CC = gcc

LDFLAGS = 
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -Wno-pedantic -Wall -O3 -g  $(CPPFLAGS)
# Source files and output
SRCS = main.c image.c
TARGET = main

# Default target
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS) 

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

db:
	gf2 $(TARGET)
