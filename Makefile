CC = clang
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -Wno-pedantic -Wall

DEBUG_FLAGS = -g -O0
RELEASE_FLAGS = -O3

LIB_DIR = ./build
SRC_DIR = ./src
LDFLAGS = -L$(LIB_DIR)/ -Wl,-rpath=$(LIB_DIR) -limglib
SRC = $(SRC_DIR)/image.c
HEADER = $(SRC_DIR)/image.h
SHARED_LIB = $(LIB_DIR)/libimglib.so
EXAMPLE_SRC = main.c
EXAMPLE_TARGET = main

all: release
lib: release

debug: $(SRC) $(HEADER)
	mkdir -p $(LIB_DIR)
	@echo "--------------------------------------------------------"
	@echo "Building: Debug shared library ($(SHARED_LIB))"
	@echo -e "--------------------------------------------------------"
	@if $(CC) --version | grep -i clang > /dev/null; then \
		$(CC) $(CPPFLAGS) $(SRC) $(CFLAGS) $(DEBUG_FLAGS) -fsanitize=memory -shared -fPIC -o $(SHARED_LIB); \
	else \
		$(CC) $(CPPFLAGS) $(SRC) $(CFLAGS) $(DEBUG_FLAGS) -shared -fPIC -o $(SHARED_LIB); \
	fi
	@echo ""

release: $(SRC) $(HEADER)
	mkdir -p $(LIB_DIR)
	@echo "--------------------------------------------------------"
	@echo "Building: Release shared library ($(SHARED_LIB))"
	@echo "--------------------------------------------------------"
	$(CC) $(CPPFLAGS) $(SRC) $(CFLAGS) $(RELEASE_FLAGS) -shared -fPIC -o $(SHARED_LIB)

example: release
	@echo "--------------------------------------------------------"
	@echo "Building: Example ($(EXAMPLE_TARGET))"
	@echo "--------------------------------------------------------"
	$(CC) $(CPPFLAGS) $(CFLAGS) $(EXAMPLE_SRC) -o $(EXAMPLE_TARGET) $(LDFLAGS)

clean:
	rm -f $(SHARED_LIB) $(EXAMPLE_TARGET)

.PHONY: all lib debug release example clean
