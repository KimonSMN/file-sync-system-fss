#ifndef GLOBALS_H
#define GLOBALS_H

#include "queue.h"
#include "sync_info_mem_store.h"

extern int active_workers;
extern int worker_count;
extern queue* q;
extern hashTable* table;
#endif
