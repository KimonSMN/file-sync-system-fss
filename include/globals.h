#ifndef GLOBALS_H
#define GLOBALS_H

#include "queue.h"
#include "sync_info_mem_store.h"

// Global variables.

extern int active_workers; // Number of curently active workers.
extern int worker_count;   // Number of MAX workers passed though the arguments when calling fss_manager/
extern queue* q;           // The Queue where operations are queued until there are available workers.
extern hashTable* table;   // Sync_info_mem_store. HashTable.
#endif
