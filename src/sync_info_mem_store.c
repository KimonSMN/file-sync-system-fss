#include <stdio.h>
#include "sync_info_mem_store.h"

// Hash function
unsigned long hash(const char *source_dir){
    unsigned long hash = 5381;
    int c;
    while ((c = *source_dir++))
        hash = ((hash << 5) + hash) + c;
    return hash % HASH_TABLE_SIZE;
}
