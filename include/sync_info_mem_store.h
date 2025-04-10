#ifndef SYNC_INFO_MEM_STORE
#define SYNC_INFO_MEM_STORE

#include <time.h>

#define HASH_TABLE_SIZE 1572869
 
// int prime_sizes[] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241,
// 	786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741};


typedef struct watchDir{
    char* source_dir;       // Source directory Path.
    char* target_dir;       // Target directory Path.
    //status;               // ?
    time_t last_sync_time;  // Time of last sync.
    int active;             // 1 watching, 0 nothing
    int error_count;        // Number of errors occured during sync

    struct watchDir* next;  // Seperate chaining
} watchDir;

typedef struct {
    watchDir* buckets[HASH_TABLE_SIZE];
} hashTable;

hashTable* init_hash_table();

void insert_watchDir(hashTable* table, watchDir* dir);

watchDir* find_watchDir(hashTable* table, const char* source_dir);

int remove_watchDir(hashTable* table, const char* source_dir);

void print_hash_table(hashTable* table);

void destroy_hash_table(hashTable* table);


#endif