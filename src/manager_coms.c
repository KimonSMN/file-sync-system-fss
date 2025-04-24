#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#include "manager_coms.h"
#include "utility.h"
#include "sync_info_mem_store.h"
#include "queue.h"

#include "globals.h"

/*                           Εντολές που µπορεί να δεχθεί ο fss_manager                         */

int manager_add(char* source, char* target, int inotify_fd, hashTable* table, queue* q){
    watchDir* found = find_watchDir(table,source);  // Try to find source directory.
    if (found != NULL) {
        if (strcmp(found->target_dir, target) != 0) {   // If the found->target_dir (original) isn't the same as the one passed, return 1 since we want one to one pair.
            printf("%s is already has a pair \n", source);
            return 1;
        }
    }
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);  

    if (found != NULL && strcmp(found->source_dir, source) == 0 && strcmp(found->target_dir, target) == 0) {    // If source directory already exists.
        printf("[%d-%02d-%02d %02d:%02d:%02d] Already in queue: %s\n",                      // Print the message.
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
            source);
        return 0;
    }

    if (found == NULL) {                    // If not found.
        found = create_dir(source, target); // Create it.
        insert_watchDir(table, found);      // Insert it to hashtable.
        found->watchdesc = inotify_add_watch(inotify_fd, source, IN_CREATE | IN_MODIFY | IN_DELETE);
        if (found->watchdesc == -1) {
            perror("Error adding inotify_watch.");
        } else {
            found->active = 1;
        }
        
    } 
    
    // If the source directory was NULL and we created it.
    FILE *fp = fopen(MANAGER_LOG_PATH, "a");    // Open the manager-log-file.
    if (!fp) {
        perror("Error opening File.");
        return 1;
    }
    

    if (active_workers < worker_count) {        // If there are available workers.
        pid_t pid = fork();

        if (pid == 0) { // Child process.
            char *args[] = {WORKER_PATH, source, target, "ALL", "FULL", NULL};  // Full Sync.
            execvp(args[0], args);

            perror("Error execvp Failed.");
        } else if (pid > 0) { // Parent process.
            active_workers++;

            printf_fprintf(fp,"[%d-%02d-%02d %02d:%02d:%02d] Added directory: %s -> %s\n",
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                source, target);
            printf_fprintf(fp,"[%d-%02d-%02d %02d:%02d:%02d] Monitoring started for %s\n", 
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                source);


        } else {
            perror("Error fork Failed.");
        }
    } else {    // If no available workers.
        node* job = init_node(source, target, "ALL", "FULL");
        enqueue(q, job);
        printf("JOB [%s -> %s] QUEUED\n", source, target);
    }

    fclose(fp);
    return 0;
}


int manager_cancel(char* source, int inotify_fd, hashTable* table){
    watchDir* found = find_watchDir(table, source); 

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    if (found == NULL || found->active == 0) {   // If Not watched.
        printf("[%d-%02d-%02d %02d:%02d:%02d] Directory not monitored: %s\n",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
            source); 
        return 0;
    }
    if(inotify_rm_watch(inotify_fd, found->watchdesc) == 0) {   // Success returns zero.
        
        FILE *fp = fopen(MANAGER_LOG_PATH, "a");    // Open the manager-log-file.
        if (!fp) {
            perror("Error opening File.");
            return 1;
        }

        found->active = 0; // set to not watch it.
        
        printf_fprintf(fp,"[%d-%02d-%02d %02d:%02d:%02d] Monitoring stopped for: %s\n",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
            source);
    
        fclose(fp);
        return 0;
    } else {
        perror("Error inotify_remove Failed.");
    }
    return 1;
}


int manager_status(char* source, hashTable* table){
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    watchDir* found = find_watchDir(table, source);
    if (found == NULL || found->active == 0) { // directory not active OR doesn't exist.
        printf("[%d-%02d-%02d %02d:%02d:%02d] Directory not monitored: %s\n",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
            source);
        return 1;
    }

    printf("[%d-%02d-%02d %02d:%02d:%02d] Status requested for %s\nDirectory: %s\nTarget: %s\nLast Sync: %ld\nErrors: %d\nStatus: %s\n",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
        found->source_dir, found->source_dir, found->target_dir, found->last_sync_time, found->error_count, found->active ? "Active":"Inactive");


    return 0;
}