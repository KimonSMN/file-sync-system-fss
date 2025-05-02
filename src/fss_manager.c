#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <limits.h>

#include <signal.h>
#include <poll.h> 

#include "sync_info_mem_store.h"
#include "queue.h"
#include "utility.h"
#include "manager_coms.h"

#include "globals.h"
#include <bits/types/sigset_t.h>
#include <bits/sigaction.h>


void handler(){ // SIGCHLD
    int status;
    node* job;
    watchDir* found;

    while((waitpid(-1, &status, WNOHANG)) > 0 ) {
        active_workers--; 

        if(isEmpty(q) == 0) { // If the queue is not empty
            job = dequeue(q); // Get the first queued job.
        
            pid_t pid = fork();
            if (pid == 0) { 
                // Child Process
                char* args[] = {WORKER_PATH, job->source_dir, job->target_dir, job->filename, job->operation, NULL};
                execvp(args[0], args);

            } else if(pid > 0) { 
                // Parent Proccess 
                active_workers++;

                char buffer[1024];
                // Pass the message to the buffer:
                snprintf(buffer, sizeof(buffer),"[%s] Added directory: %s -> %s\n[%s] Monitoring started for %s\n", get_time(), job->source_dir, job->target_dir, get_time(), job->source_dir);

                write(STDOUT_FILENO,buffer, strlen(buffer));                                    // Log to the terminal
                int manager_fd = open(manager_log_path, O_WRONLY | O_CREAT | O_APPEND, 0777);   // Open the manager-log.
                write(manager_fd, buffer, strlen(buffer));                                      // Log the message to it.
   
                found = find_watchDir(table, job->source_dir);  // Find the job's `source` directory.
                if (found != NULL) {
                    found->last_sync_time = time(NULL);         // Set last sync time to current time.
                }
                destroy_node(job);  // Free memory.
                close(manager_fd);  // Close manager-log. 
            }
        }
    }
}


int main(int argc, char* argv[]){

    signal(SIGCHLD, handler);

    // Initialize variables.
    char* manager_log = NULL;
    char* config_file = NULL;
    active_workers = 0;

    // Flags.
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            manager_log = argv[++i];
        } else if (strcmp(argv[i], "-c") == 0) {
            config_file = argv[++i];
        } else if (strcmp(argv[i], "-n") == 0){
            worker_count = atoi(argv[++i]);
        }
    }

    // Sanity check.
    if (manager_log == NULL || config_file == NULL || worker_count < MAX_WORKERS) {
        printf("Usage: ./fss_manager -l <manager_logfile> -c <config_file> -n <worker_count>\n");
        return 1;
    }

    set_path_manager(manager_log, config_file); // Update the manager-log & config files, to the paths provided in the flags -l, -c.

    // Create necessary named-pipes:
    create_named_pipe(FIFO_IN);
    create_named_pipe(FIFO_OUT);

    // Open the named-pipes:
    int fss_in = open(FIFO_IN, O_RDONLY | O_NONBLOCK);
    if (fss_in == -1)
        return 1;
    

    int fss_out = open(FIFO_OUT, O_RDWR | O_NONBLOCK);
    if (fss_out == -1)
        return 1;
    
    table = init_hash_table();  // Initialize hash table.
    q = init_queue();           // Initialize Queue.

    // Initialize the signal mask.
    sigset_t block_child;
    sigemptyset(&block_child);
    sigaddset(&block_child, SIGCHLD);
    sigprocmask(SIG_BLOCK, &block_child, NULL);
    
    FILE *fp = fopen(config_path, "r");         // Open the config file for reading.
    if (fp == NULL) {   // Sanity Check.
        perror("Error opening file.");
        return 1;
    }

    FILE *mlfp = fopen(manager_log_path, "w");  // Open the config file for writing.
    if (mlfp == NULL) {    // Sanity Check.
        perror("Error opening file.");
        return 1;
    }

    // Initialize inotifiy instance. 
    int inotify_fd = inotify_init();

    // Initialize variables.
    char line[1024], source_dir[512], target_dir[512];
    
    while (fgets(line, sizeof(line), fp)) { // Read the config file. fp is the file pointer of config-file.
        line[strcspn(line, "\n")] = 0;

        char *token = strtok(line, " ");    // Tokenize the lines.
        if(token != NULL) {                 
            strcpy(source_dir, token);      // First token is source.

            token = strtok(NULL,"");
            if(token != NULL) {
                strcpy(target_dir, token);  // Second token is target.
            } else {
                target_dir[0] = '\0';
                perror("Error with <source> <target> in Config file.");
            }
        }
        if (check_dir(source_dir) == 0 && check_dir(target_dir) == 0) { // Inserts valid directories to hash-table.

            watchDir* curr = create_dir(source_dir, target_dir);    // Creates directory.
            insert_watchDir(table, curr);                           // Inserts to table.
            
            // Start watching directory.
            curr->active = 1;                   // Set current directory to active. 
            curr->watchdesc = inotify_add_watch(inotify_fd, source_dir, IN_CREATE | IN_MODIFY | IN_DELETE); // Add it to inotify watch list.
            curr->last_sync_time = time(NULL);  // Set the last_sync_time to the current time.

            if (active_workers < worker_count){ // If there are available workers:
                
                spawn_worker(curr->source_dir,curr->target_dir, mlfp, "ALL", "FULL");   // Start a worker with full sync.

            } else { // If there are no available workers:
                node* job = init_node(curr->source_dir, curr->target_dir, "ALL", "FULL"); // Initialize the job.
                enqueue(q, job); // Enqueue it.
            }
        }
    }

    fclose(mlfp);   // Close manager-log.
    fclose(fp);     // Close config file.

    char buffer[BUFFER_SIZE];
    struct pollfd fds[2];

    fds[0].fd = inotify_fd;
    fds[0].events = POLLIN;

    fds[1].fd = fss_in;
    fds[1].events = POLLIN;

    sigprocmask(SIG_UNBLOCK, &block_child, NULL);

    while (1) { // Infinite loop.
        poll(fds, 2, -1);   // Wait for one of the fds to become ready, forever.

        // Inotify events.
        if (fds[0].revents & POLLIN) {  // If the inotfiy_fd has data to read:
            ssize_t bytesRead_inotify = read(inotify_fd, buffer, sizeof(buffer)); // Read into the buffer.

            for (char* ptr = buffer; ptr < buffer + bytesRead_inotify;){
                struct inotify_event* event = (struct inotify_event *) ptr;
                if (event->len > 0) {
                    watchDir* found = find_watchDir_wd(table, event->wd);   // Find the watchedDir by the provided watch descriptor.
                    if (found == NULL) {
                        ptr += sizeof(struct inotify_event) + event->len;
                        continue;
                    }
                    char* opt;  // Variable to store operation.
                    if (event->mask & IN_CREATE) {          // Create file.
                        printf("IN_CREATE: %s\n", event->name);
                        opt = "ADDED";
                    } else if (event->mask & IN_MODIFY) {   // Modify file.
                        printf("IN_MODIFY: %s\n", event->name);
                        opt = "MODIFIED";
                    } else if (event->mask & IN_DELETE) {   // Delete file.
                        printf("IN_DELETE: %s\n", event->name);
                        opt = "DELETED";
                    }

                    if (active_workers < worker_count){ // If there are available workers:

                        spawn_worker(found->source_dir, found->target_dir, NULL, event->name, opt);
        
                    } else { // Queue job:
                        node* job = init_node(found->source_dir, found->target_dir, event->name, opt);
                        enqueue(q, job);
                    }
                }
                ptr += sizeof(struct inotify_event) + event->len;
            }
        }

        // Console commands.
        if (fds[1].revents & POLLIN) {  // If fss_in pipe has data to read:
            ssize_t bytesRead_fss_in = read(fss_in, buffer, sizeof(buffer) - 1);   // Read into the buffer.
            if (bytesRead_fss_in > 0) {
                buffer[bytesRead_fss_in] = '\0';
         
                // Tokenize the buffer:
                char* command = strtok(buffer," ");
                char* source = strtok(NULL," ");
                char* target = strtok(NULL," ");
                
                if (strcmp(command, "add") == 0) {
                    manager_add(source, target, inotify_fd, table, q, fss_out);
                } else if (strcmp(command, "cancel") == 0) {
                    manager_cancel(source,inotify_fd, table, fss_out);
                } else if (strcmp(command, "status") == 0) {
                    manager_status(source, table, fss_out);
                } else if (strcmp(command, "sync") == 0) {
                    manager_sync(source, table, inotify_fd, fss_out);
                } else {
                    manager_shutdown(table,inotify_fd, fss_out);
                    break;
                }
            } else if (bytesRead_fss_in == 0) {
                close(fss_in);
                fss_in = open(FIFO_IN, O_RDONLY | O_NONBLOCK);
                fds[1].fd = fss_in;
            }
        }
    }

    // Close named-pipes:
    close(fss_in);
    close(fss_out); 

    // Free memory:
    destroy_hash_table(table);
    delete_named_pipe(FIFO_IN);
    delete_named_pipe(FIFO_OUT);

    return 0;
}
