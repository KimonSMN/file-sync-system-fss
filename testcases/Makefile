# Compiler and flags
CC = gcc
CFLAGS = -Wall -g
INCLUDES = -I../include

# Paths
SRC_DIR = ../src
BUILD_DIR = ../testcases

# Source files
TEST_SRC = test_sync_info.c
COMMON_SRC = $(SRC_DIR)/sync_info_mem_store.c

# Output binary
TEST_BIN = $(BUILD_DIR)/test_sync_info

# Default target
all: $(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) $(COMMON_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Run tests
run: $(TEST_BIN)
	./$(TEST_BIN)

# Cleanup
clean:
	rm -f $(TEST_BIN)

.PHONY: all run clean
