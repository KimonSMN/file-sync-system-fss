# Compiler and flags
CC = gcc
CFLAGS = -Wall -g
INCLUDES = -Iinclude

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files
MANAGER_SRC = $(SRC_DIR)/fss_manager.c
CONSOLE_SRC = $(SRC_DIR)/fss_console.c
WORKER_SRC  = $(SRC_DIR)/worker.c

# Output binaries
MANAGER_BIN = $(BUILD_DIR)/fss_manager
CONSOLE_BIN = $(BUILD_DIR)/fss_console
WORKER_BIN  = $(BUILD_DIR)/worker

# Default target
all: $(MANAGER_BIN) $(CONSOLE_BIN) $(WORKER_BIN)

# Build manager
$(MANAGER_BIN): $(MANAGER_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Build console
$(CONSOLE_BIN): $(CONSOLE_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Build worker
$(WORKER_BIN): $(WORKER_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Clean up
clean:
	rm -f $(BUILD_DIR)/*

# Phony targets
.PHONY: all clean
