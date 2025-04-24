#ifndef COMS
#define COMS

#include "sync_info_mem_store.h"
#include "queue.h"

int manager_add(char* source, char* target, int inotify_fd, hashTable* table, queue* q);

int manager_cancel(char* source, int inotify_fd, hashTable* table);

int manager_status(char* source, hashTable* table);

int manager_sync(char* source, hashTable* table,int inotify_fd);

#endif