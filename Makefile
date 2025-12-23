include config.mk

CC = clang
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -Wno-pedantic -Wall

DEBUG_FLAGS = -gggdb -O0
RELEASE_FLAGS = -O3 -ggdb

BUILD_DIR = ./build
SRC_DIR = ./src

LDFLAGS = -L$(BUILD_DIR)/ -Wl,-rpath=$(BUILD_DIR) -limglib
SHARED_LIB = $(BUILD_DIR)/libimglib.so
ARENA_OBJ = $(BUILD_DIR)/arena.o
EXAMPLE_TARGET = main

# Source Files
EXAMPLE_SRC = main.c
IMAGE_SRC = $(SRC_DIR)/image.c
ARENA_SRC = $(SRC_DIR)/arena.c

# Headers
IMAGE_H = $(SRC_DIR)/image.h
ARENA_H = $(SRC_DIR)/arena.h

HEADERS = $(IMAGE_H) $(ARENA_H)

all: deps release
lib: deps release


deps:
	@if [ ! -f "$(ARENA_H)" ] || \
		[ ! -f "$(ARENA_SRC)" ] ; then \
		echo "==> Dependencies not found, fetching..."; \
		$(MAKE) fetch; \
	fi


# Arena must be compiled before string (dependency)
$(ARENA_OBJ): $(ARENA_SRC) $(ARENA_H)
	mkdir -p $(BUILD_DIR)
	@echo "==> Compiling: $(ARENA_SRC)"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $(ARENA_SRC) -o $(ARENA_OBJ)


fetch:
	@echo "==> Fetching dependencies..."
	@if [ ! -d "$(SRC_DIR)" ]; then mkdir -p $(SRC_DIR); fi
	@echo "==> Downloading arena library..."
	curl -L "$(ARENA_URL)/arena.c" -o $(ARENA_SRC) || \
		{ echo "Failed to download arena.c"; exit 1; }
	curl -L "$(ARENA_URL)/arena.h" -o $(ARENA_H) || \
		{ echo "Failed to download arena.h"; exit 1; }
	@echo "==> Dependencies fetched successfully"

debug: $(ARENA_OBJ) $(IMAGE_SRC) $(HEADERS) 
	mkdir -p $(BUILD_DIR)
	@echo "--------------------------------------------------------"
	@echo "Building: Debug shared library ($(SHARED_LIB))"
	@echo -e "--------------------------------------------------------"
	@if $(CC) --version | grep -i clang > /dev/null; then \
		$(CC) $(CPPFLAGS) $(ARENA_OBJ) $(IMAGE_SRC) $(CFLAGS) $(DEBUG_FLAGS) -fsanitize=memory -shared -fPIC -o $(SHARED_LIB); \
	else \
		$(CC) $(CPPFLAGS) $(ARENA_OBJ) $(IMAGE_SRC) $(CFLAGS) $(DEBUG_FLAGS) -shared -fPIC -o $(SHARED_LIB); \
	fi
	@echo ""

release: $(ARENA_OBJ) $(IMAGE_SRC) $(HEADERS)
	mkdir -p $(BUILD_DIR)
	@echo "--------------------------------------------------------"
	@echo "Building: Release shared library ($(SHARED_LIB))"
	@echo "--------------------------------------------------------"
	$(CC) $(CPPFLAGS) $(ARENA_OBJ) $(IMAGE_SRC) $(CFLAGS) $(RELEASE_FLAGS) -shared -fPIC -o $(SHARED_LIB)

example: release
	@echo "--------------------------------------------------------"
	@echo "Building: Example ($(EXAMPLE_TARGET))"
	@echo "--------------------------------------------------------"
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ARENA_OBJ) $(EXAMPLE_SRC) -o $(EXAMPLE_TARGET) $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(ARENA_SRC) $(ARENA_H)

.PHONY: all lib debug release example clean
