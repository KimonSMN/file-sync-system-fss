#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sync_info_mem_store.h"

watchDir* create_dir(char* source_dir, char* target_dir){
    watchDir* dir = malloc(sizeof(watchDir));
    dir->source_dir = strdup(source_dir);
    dir->target_dir = strdup(target_dir);
    dir->active = 1;
    dir->last_sync_time = 0;
    dir->error_count = 0;
    dir->next = NULL;
    return dir;
}


void test_insert_find(void) {
    hashTable* table = init_hash_table();

    watchDir* dir = create_dir("./dummy/docs", "./dummy/backup");

    insert_watchDir(table, dir);

    watchDir* found = find_watchDir(table, "./dummy/docs");
    if (found && strcmp(found->target_dir, "./dummy/backup") == 0)
        printf("test_insert_find.c Passed.\n");
    else
        printf("test_insert_find.c Failed.\n");

    destroy_hash_table(table);
}

void test_remove(void){
    hashTable* table = init_hash_table();

    watchDir* dir1 = create_dir("./dummy/docs", "./dummy/backup");
    watchDir* dir2 = create_dir("./test/docs", "./test/backup");

    insert_watchDir(table, dir1);
    insert_watchDir(table, dir2);

    int removed_flag = remove_watchDir(table, "./dummy/docs");
    if (removed_flag == 0)
        printf("test_remove.c Passed.\n");
    else
        printf("test_remove.c Failed.\n");

    destroy_hash_table(table);
}

int main() {
    test_insert_find();
    test_remove();
    return 0;
}
