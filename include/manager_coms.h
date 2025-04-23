#ifndef COMS
#define COMS

#include "sync_info_mem_store.h"
#include "queue.h"

int manager_add(char* source, char* target, int inotify_fd, hashTable* table, int active_workers, int worker_count, queue* q);
#endif