#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sync_info_mem_store.h"


void test_insert_find() {
    hashTable* table = init_hash_table();

    watchDir* dir = malloc(sizeof(watchDir));
    dir->source_dir = strdup("./dummy/docs");
    dir->target_dir = strdup("./dummy/backup");
    dir->active = 1;
    dir->last_sync_time = 0;
    dir->error_count = 0;
    dir->next = NULL;

    insert_watchDir(table, dir);

    watchDir* found = find_watchDir(table, "./dummy/docs");
    if (found && strcmp(found->target_dir, "./dummy/backup") == 0)
        printf("passed\n");
    else
        printf("fail\n");

    destroy_hash_table(table);
}

int main() {
    test_insert_find();
    return 0;
}
