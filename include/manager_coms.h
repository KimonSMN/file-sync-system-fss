#ifndef COMS
#define COMS

#include "sync_info_mem_store.h"
#include "queue.h"


//                       Commands that the fss_manager can receive.                       //

/* Inserts `source` directory for monitoring and immediately starts syncing of its contents to `target` dir.*/
int manager_add(char* source, char* target, int inotify_fd, hashTable* table, queue* q, int fss_out);

/* Cancels monitoring of the `source` directory. */
int manager_cancel(char* source, int inotify_fd, hashTable* table, int fss_out);

/* Returns information about the sync status of the `source` directory. */
int manager_status(char* source, hashTable* table, int fss_out);

/* Starts syncing of the `source` directory. Regardless of whether changes have been detected. */
int manager_sync(char* source, hashTable* table, int inotify_fd, int fss_out);

/* 1. Stops monitoring of directories, 
    2. Waits for active worker processes to end,
    3. Processes any remaining tasks in the queue, 
    4. Shuts down. */
int manager_shutdown(hashTable* table, int inotify_fd, int fss_out);

#endif