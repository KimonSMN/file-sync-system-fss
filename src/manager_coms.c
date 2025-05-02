#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "manager_coms.h"
#include "utility.h"
#include "sync_info_mem_store.h"
#include "queue.h"
#include "globals.h"


int manager_add(char* source, char* target, int inotify_fd, hashTable* table, queue* q, int fss_out){   //
    
    char buffer[BUFFER_SIZE_SMALL];                                 // Allocate memory for the buffer.

    watchDir* found = find_watchDir(table,source);                  // Try to find the 'source' dir.
    if (found != NULL) {                                            // If found:
        if (strcmp(found->target_dir, target) != 0) {               // Has a different target pair than the one in `target`.
            printf("[%s] Already in queue: %s\n",get_time(),source);// Log to the terminal.
            if (fss_out > 0) {
                // Send message back to the pipe:
                snprintf(buffer, sizeof(buffer),"[%s] Already in queue: %s\n", get_time(), source);
                write(fss_out, buffer, strlen(buffer));
            }
            return 0;
        }
    }

    if (found != NULL && strcmp(found->target_dir, target) == 0) {  // If found && the found target == provided target.
        printf("[%s] Already in queue: %s\n", get_time(), source);    // Log to terminal.
        if (fss_out > 0) {  
            // Send message back to the pipe:
            snprintf(buffer, sizeof(buffer),"[%s] Already in queue: %s\n",get_time(),source);
            write(fss_out, buffer, strlen(buffer));
        }
        return 0;   // Return If `source` already in queue.
    }

    // If `found` isn't actually found (contrary to its name):

    if (found == NULL) {
        found = create_dir(source, target);                         // Create it.
        insert_watchDir(table, found);                              // Insert it to the hash-table.
        found->watchdesc = inotify_add_watch(inotify_fd, source, IN_CREATE | IN_MODIFY | IN_DELETE);    // Add to watch.
        if (found->watchdesc == -1) {                               // If inotify failed:
            perror("Error adding inotify_watch.");                  // Error msg.
            found->error_count++;                                   // Increment the error count.
            remove_watchDir(table, found->source_dir);              // Remove the directory from the hash_table.
            return 1;
        } else {
            found->active = 1;                                      // If inotify didn't fail set found's active to 1.
        }
        
    } 
    
    FILE *fp = fopen(manager_log_path, "a");                // Open the manager-log.
    if (fp == NULL) {                                       // Error check:
        perror("Error opening File.");
        if (found != NULL)
            found->error_count++;                           // Increment error count.
        return 1;
    }
    
    if (active_workers < worker_count) {                          // If there are available workers:
        if (spawn_worker(source, target, fp, "ALL", "FULL") != 0) // Spawn a worker. If this failed increment error count.  
            found->error_count++;

        if (fss_out > 0) {
            // Send message back to the pipe: (This message was produced in spawn_worker).
            snprintf(buffer, sizeof(buffer),"[%s] Added directory: %s -> %s\n[%s] Monitoring started for %s\n",
                get_time(),source, target, get_time(),source);
            write(fss_out, buffer, strlen(buffer));
        }
    } else {                                                    // If there are no available workers:
        node* job = init_node(source, target, "ALL", "FULL");   // Create a job.
        enqueue(q, job);                                        // Enqueue it.
    }

    fclose(fp); // Close manager-log
    return 0;
}


int manager_cancel(char* source, int inotify_fd, hashTable* table, int fss_out){
   
    watchDir* found = find_watchDir(table, source); 
    char buffer[1024];

    if (found == NULL || found->active == 0) {                              // If `source` directory isn't being spied on:
        printf("[%s] Directory not monitored: %s\n", get_time(),source);    // Log to the terminal.
        if (fss_out > 0) {
            // Send message back to the pipe:
            snprintf(buffer, sizeof(buffer),"[%s] Directory not monitored: %s\n", get_time(), source); 
            write(fss_out, buffer, strlen(buffer));
        }
        return 0;
    }
    if (inotify_rm_watch(inotify_fd, found->watchdesc) == 0) {   // If inotify_rm_watch succeeded.

        FILE *fp = fopen(manager_log_path, "a");                // Open the manager-log.
        if (fp == NULL) {                                       // Error check:
            perror("Error opening File.");
            if (found != NULL)
                found->error_count++;
            return 1;
        }

        found->active = 0; // Set active to 0 = Directory isn't being monitored.
        
        printf_fprintf(fp,"[%s] Monitoring stopped for %s\n", get_time(), source);  // Log to terminal and to manager-log.
        if (fss_out > 0) {
            // Send message back to the pipe:
            snprintf(buffer, sizeof(buffer),"[%s] Monitoring stopped for %s\n", get_time(), source);
            write(fss_out, buffer, strlen(buffer));
        }
        fclose(fp);     // Close manager-log.
        return 0;
    } else {                                                // If inotify_rm_watch failed.
        perror("Error inotify_remove Failed.");
        found->error_count++;
    }
    return 1;
}


int manager_status(char* source, hashTable* table, int fss_out){
    
    watchDir* found = find_watchDir(table, source);
    char buffer[1024];

    if (found == NULL) {        // Never been watched / Not in our hash-table.
        printf("[%s] Directory not monitored: %s\n", get_time(), source);   // Log to terminal.
        if (fss_out > 0) {
            // Send message back to the pipe:
            snprintf(buffer, sizeof(buffer),"[%s] Directory not monitored: %s\n", get_time(), source);
            write(fss_out, buffer, strlen(buffer));
        }  
        return 1;
    }

    // Get last sync time of `source` diretory.
    struct tm tm = *localtime(&found->last_sync_time);
    char last_sync_time[32];    // This is the string the time will be passed to.
    strftime(last_sync_time, sizeof(last_sync_time), "%Y-%m-%d %H:%M:%S", &tm);

    printf("[%s] Status requested for %s\nDirectory: %s\nTarget: %s\n", get_time(), found->source_dir, found->source_dir, found->target_dir); // Log to terminal.

    if (found->last_sync_time == 0) {               // No last sync time found.
        printf("Last Sync: ---\n");
    } else {
        printf("Last Sync: %s\n",last_sync_time);   // If found log it to the terminal.
    }    
    printf("Errors: %d\nStatus: %s\n",found->error_count, found->active ? "Active":"Inactive"); // Log to terminal.

    if (fss_out > 0) {
        // Send message back to the pipe:
        snprintf(buffer, sizeof(buffer), "[%s] Status requested for %s\nDirectory: %s\nTarget: %s\n", get_time(), found->source_dir, found->source_dir, found->target_dir);
        write(fss_out, buffer, strlen(buffer));
        
        if (found->last_sync_time == 0) {
            snprintf(buffer, sizeof(buffer), "Last Sync: ---\n");
            write(fss_out, buffer, strlen(buffer));
        } else {
            snprintf(buffer, sizeof(buffer), "Last Sync: %s\n", last_sync_time);
            write(fss_out, buffer, strlen(buffer));
        }
        
        snprintf(buffer, sizeof(buffer), "Errors: %d\nStatus: %s\n", found->error_count, found->active ? "Active" : "Inactive");
        write(fss_out, buffer, strlen(buffer));
    }  

    return 0;
}

int manager_sync(char* source, hashTable* table,int inotify_fd, int fss_out) {
   
    watchDir* found = find_watchDir(table, source);
    char buffer[1024];

    if (found == NULL) {    // There was nothing mentioned about what to print so I print this:
        printf("[%s] Directory not monitored: %s\n", get_time(), source); // Log to terminal.
        if (fss_out > 0) {
            // Send message back to the pipe:
            snprintf(buffer, sizeof(buffer),"[%s] Directory not monitored: %s\n", get_time(), source);
            write(fss_out, buffer, strlen(buffer));
        }     
        return 1;
    }

    // Not implemented
    // if (found->syncing == 1) {  // If already syncing:
    //     printf("[%s] Sync already in progress %s\n", get_time(), found->source_dir);
    //     return 0;
    // }

    char* target = found->target_dir;
    
    FILE *fp = fopen(manager_log_path, "a");    // Open manager-log.
    if (fp == NULL) {                           // Error check:
        perror("Error opening file.");
        if (found != NULL)
            found->error_count++;
        return 1;
    }

    if (active_workers < worker_count){         // If there are available active workers:
        pid_t pid = fork(); 

        if (pid == 0) {
            // Child process.

            char *args[] = {WORKER_PATH, source, target, "ALL", "FULL", NULL};  // Execute job with Full sync.
            execvp(args[0], args);

            perror("Error execvp Failed.");                                     // If execvp failed:
            exit(1);                                                            // Exit.
        } else if (pid > 0) {
            // Parent process.

            active_workers++;   // Increase worker count.
            printf_fprintf(fp,"[%s] Syncing directory: %s -> %s\n", get_time(), source, target);    // Log to terminal & to manager-log.
            if (fss_out > 0) {
                // Send message back to the pipe:
                snprintf(buffer, sizeof(buffer),"[%s] Syncing directory: %s -> %s\n", get_time(), source, target);
                write(fss_out, buffer, strlen(buffer));
            }  
            found->active = 1;                                              // "Syncing directory" is equal to "Monitoring started".
            found->last_sync_time = time(NULL);                             // Update the last time of sync.
            found->watchdesc = inotify_add_watch(inotify_fd, source, IN_CREATE | IN_MODIFY | IN_DELETE);    // Start monitoring the directory.

            // Not implemented
            // This has to be moved once to once the worker finishes his job.
            // printf_fprintf(fp,"[%s] Sync completed: %s -> %s Errors: %d\n", 
            //     get_time(),
            //     source,target, found->error_count);

        } else {
            perror("Error fork Failed.");
            found->error_count++;
            exit(1);
        }
    } else { // If no available workers:
        node* job = init_node(source, target, "ALL", "FULL"); // Initialize job.
        enqueue(q, job);                                      // Enqueue it.
    }
    return 0;
}

int manager_shutdown(hashTable* table, int inotify_fd, int fss_out) {
    char buffer[2056];

    for (int i = 0; i < HASH_TABLE_SIZE; i++) {                 // For every directory in the hash-table.
        watchDir* curr = table->buckets[i];
        while (curr != NULL) {
            if (curr->active == 1 && curr->watchdesc >= 0) {    // If directory is being monitored:
                inotify_rm_watch(inotify_fd, curr->watchdesc);  // Remove it from the inotify list.
                curr->active = 0;                               // Set active to 0.
            }
            curr = curr->next;                              // Go to the next one.
        }
    }

    printf("[%s] Shutting down manager...\n", get_time());

    printf("[%s] Waiting for all active workers to finish.\n", get_time());

    printf("[%s] Processing remaining queued tasks.\n", get_time());

    while (active_workers > 0) {    // While there are active workers, so there are active jobs, pause.
        pause();
    }

    printf("[%s] Manager shutdown complete.\n", get_time());

    if (fss_out > 0) {
        // Send message back to the pipe:

        snprintf(buffer, sizeof(buffer), "[%s] Shutting down manager...\n", get_time());
        write(fss_out, buffer, strlen(buffer));

        snprintf(buffer, sizeof(buffer), "[%s] Waiting for all active workers to finish.\n", get_time());
        write(fss_out, buffer, strlen(buffer));

        snprintf(buffer, sizeof(buffer), "[%s] Processing remaining queued tasks.\n", get_time());
        write(fss_out, buffer, strlen(buffer));

        snprintf(buffer, sizeof(buffer), "[%s] Manager shutdown complete.\n", get_time());
        write(fss_out, buffer, strlen(buffer));
    }  

    return 0;
}