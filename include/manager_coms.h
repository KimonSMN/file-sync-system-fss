#ifndef COMS
#define COMS

#include "sync_info_mem_store.h"

int manager_add(char* source, char* target, int inotify_fd, hashTable* table);

#endif