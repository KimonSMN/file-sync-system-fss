#include <stdio.h>
#include <string.h>
#include "sync_info_mem_store.h"

/* djb2 hash function. */
unsigned long hash(const char *source_dir){
    unsigned long hash = 5381;
    int c;
    while ((c = *source_dir++))
        hash = ((hash << 5) + hash) + c;
    return hash % HASH_TABLE_SIZE;
}


/* Initialize an empty hash-table. */
hashTable* init_hash_table(){
    hashTable* table = malloc(sizeof(hashTable));
    if(table == NULL){
        return NULL;
    }
    
    for(int i = 0; i < HASH_TABLE_SIZE; i++) {
        table->buckets[i] = NULL;
    }
    return table;
}

/* Insert to hash-table the specified dir. */
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

/* Search for source_dir (key) inside hash-table.
    If found, return the directory.
    Else return NULL.  */
watchDir* find_watchDir(hashTable* table, const char* source_dir){

    if (table == NULL || source_dir == NULL) {
        return NULL;
    }

    int index = hash(source_dir);

    watchDir* found = table->buckets[index];
    while (found) {
        if(strcmp(found->source_dir, source_dir) == 0) {
            return found;
        }
        found = found->next;
    }

    return NULL;
}

/* Remove source_dir from hash-table.
    If success return 0.
    Else return 1.*/
int remove_watchDir(hashTable* table, const char* source_dir){
    watchDir* to_remove = table->buckets[hash(source_dir)];
    watchDir* prev = NULL;

    while (to_remove) {
        if (strcmp(to_remove->source_dir, source_dir) == 0){
            if(prev) {
                prev->next = to_remove->next;
            } else {
                table->buckets[hash(source_dir)] = to_remove->next;
            }
            free(to_remove->source_dir);
            free(to_remove->target_dir);
            free(to_remove);
            return 0;  // Success
        }
        prev = to_remove;
        to_remove = to_remove->next;
    }
    return 1;
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

/* Free allocated memory of hash-table. */
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

/* Create a watchDir instance and return it. */
watchDir* create_dir(char* source_dir, char* target_dir){
    watchDir* dir = malloc(sizeof(watchDir));
    dir->source_dir = strdup(source_dir);
    dir->target_dir = strdup(target_dir);
    dir->active = 0;
    dir->last_sync_time = 0;
    dir->error_count = 0;
    dir->next = NULL;
    dir->watchdesc = 0;
    return dir;
}