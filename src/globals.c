#include "globals.h"

int active_workers = 0; // Initialy zero.
int worker_count = 5;   // Default is 5 unless specified by flag in fss_manager.
queue* q;               // Global queue.
hashTable* table;       // Global hash-table.
