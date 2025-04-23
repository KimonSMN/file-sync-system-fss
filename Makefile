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
WORKER_SRC = $(SRC_DIR)/worker.c
SYNC_SRC = $(SRC_DIR)/sync_info_mem_store.c
QUEUE_SRC = $(SRC_DIR)/queue.c 
COMS_SRC = $(SRC_DIR)/manager_coms.c 
UTILITY_SRC = $(SRC_DIR)/utility.c 

# Output binaries
MANAGER_BIN = $(BUILD_DIR)/fss_manager
CONSOLE_BIN = $(BUILD_DIR)/fss_console
WORKER_BIN = $(BUILD_DIR)/worker
COMS_BIN = $(BUILD_DIR)/manager_coms
UTILITY_BIN = $(BUILD_DIR)/utility

# Default target
all: $(MANAGER_BIN) $(CONSOLE_BIN) $(WORKER_BIN)

# Build manager
$(MANAGER_BIN): $(MANAGER_SRC) $(SYNC_SRC) $(QUEUE_SRC) $(COMS_SRC) $(UTILITY_SRC) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Build console
$(CONSOLE_BIN): $(CONSOLE_SRC) $(SYNC_SRC) $(QUEUE_SRC) $(COMS_SRC) $(UTILITY_SRC) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Build worker
$(WORKER_BIN): $(WORKER_SRC) $(SYNC_SRC) $(QUEUE_SRC) $(COMS_SRC) $(UTILITY_SRC) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^



# Clean up
clean:
	rm -f $(BUILD_DIR)/*

# Phony targets
.PHONY: all clean
