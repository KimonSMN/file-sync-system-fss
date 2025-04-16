# Compiler and flags
CC = gcc
CFLAGS = -Wall -g
INCLUDES = -I../include

# Paths
SRC_DIR = ../src
BUILD_DIR = ../testcases

# Source files
TEST_SYNC_SRC = test_sync_info.c
TEST_QUEUE_SRC = test_queue.c
COMMON_SRC = $(SRC_DIR)/sync_info_mem_store.c
QUEUE_SRC = $(SRC_DIR)/queue.c

# Output binaries
TEST_SYNC_BIN = $(BUILD_DIR)/test_sync_info
TEST_QUEUE_BIN = $(BUILD_DIR)/test_queue

# Default target
all: $(TEST_SYNC_BIN) $(TEST_QUEUE_BIN)

$(TEST_SYNC_BIN): $(TEST_SYNC_SRC) $(COMMON_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

$(TEST_QUEUE_BIN): $(TEST_QUEUE_SRC) $(QUEUE_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Run tests
run_sync: $(TEST_SYNC_BIN)
	./$(TEST_SYNC_BIN)

run_queue: $(TEST_QUEUE_BIN)
	./$(TEST_QUEUE_BIN)

# Cleanup
clean:
	rm -f $(TEST_SYNC_BIN) $(TEST_QUEUE_BIN)

.PHONY: all run_sync run_queue clean
