#include <stdio.h>
#include <string.h>
#include "sync_info_mem_store.h"

// Hash function
unsigned long hash(const char *source_dir){
    unsigned long hash = 5381;
    int c;
    while ((c = *source_dir++))
        hash = ((hash << 5) + hash) + c;
    return hash % HASH_TABLE_SIZE;
}


hashTable* init_hash_table(){
    hashTable* table = malloc(sizeof(hashTable));
    if(table == NULL){
        return NULL;
    }
    
    for(int i = 0; i < HASH_TABLE_SIZE; i++) {
        table->buckets[i] = NULL;       // Initialize everything to NULL
    }
    return table;
}

void insert_watchDir(hashTable* table, watchDir* dir){
    int index = hash(dir->source_dir);
    
    watchDir* curr = table->buckets[index];
    
    while (curr) {
        if (strcmp(curr->source_dir, dir->source_dir) == 0) {
            return;
        }
        curr = curr->next;
    }

    dir->next = table->buckets[index];
    table->buckets[index] = dir;

}

watchDir* find_watchDir(hashTable* table, const char* source_dir){

}

int remove_watchDir(hashTable* table, const char* source_dir){
    return 0;
}

void print_hash_table(hashTable* table){
    if (table == NULL) {
        return;
    }
    
    for(int i = 0; i < HASH_TABLE_SIZE; i++) {
        watchDir* curr = table->buckets[i];
        if (curr) {
            printf("Bucket %d:\n", i);
            while (curr) {
                printf(" Source %s - Target %s - Active %d\n", curr->source_dir, curr->target_dir, curr->active);
                curr = curr->next;
            }
        }
    }
}

void destroy_hash_table(hashTable* table){
    if (table == NULL) {
        return;
    }

    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        watchDir* curr = table->buckets[i];
        while (curr) {
            watchDir* temp = curr;
            curr = curr->next;

            free(temp->source_dir);
            free(temp->target_dir);
            free(temp);
        }
    }
    free(table);
}