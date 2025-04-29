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
    // This signal is sent to a parent process whenever one of its child processes terminates or stops.
        
    int status;
    node* job;
    struct tm tm = get_time();
    watchDir* found;

    while((waitpid(-1, &status, WNOHANG)) > 0 ) {
        active_workers--;
        if(isEmpty(q) == 0) { // if the queue isn't empty.
            job = dequeue(q); // remove the worker from it.
        
            pid_t pid = fork(); // new worker
            if (pid == 0) { // child
                char* args[] = {WORKER_PATH, job->source_dir, job->target_dir, job->filename, job->operation, NULL};
                execvp(args[0], args);

            } else if(pid > 0) { // parent
                active_workers++;

                char buffer[1024];
                snprintf(buffer, sizeof(buffer),"[%d-%02d-%02d %02d:%02d:%02d] Added directory: %s -> %s\n"
                                "[%d-%02d-%02d %02d:%02d:%02d] Monitoring started for %s\n",
                                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, job->source_dir, job->target_dir,
                                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, job->source_dir);
                
                write(STDOUT_FILENO,buffer, strlen(buffer));
                int manager_fd = open(manager_log_path, O_WRONLY | O_CREAT | O_APPEND, 0777);
                write(manager_fd, buffer, strlen(buffer)) ;

                found = find_watchDir(table, job->source_dir);
                if (found != NULL) {
                    found->last_sync_time = time(NULL);
                }

                destroy_node(job);
            }
        }
    }
}

int main(int argc, char* argv[]){

    signal(SIGCHLD, handler); //volatile sig_atomic_t maybe need to use this here?

    char* manager_log = NULL;
    char* config_file = NULL;
    active_workers = 0;

    // Flags
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            manager_log = argv[++i];
        } else if (strcmp(argv[i], "-c") == 0) {
            config_file = argv[++i];
        } else if (strcmp(argv[i], "-n") == 0){
            worker_count = atoi(argv[++i]);
        }
    }

    if (manager_log == NULL || config_file == NULL || worker_count < MAX_WORKERS) {
        printf("Usage: ./fss_manager -l <manager_logfile> -c <config_file> -n <worker_count>\n");
        return 1;
    }

    set_path_manager(manager_log, config_file);

    // Create necessary named-pipes
    create_named_pipe(FIFO_IN);
    create_named_pipe(FIFO_OUT);

    int fss_in = open(FIFO_IN, O_RDONLY | O_NONBLOCK);
    if (fss_in == -1) {
        return 1;
    }

    int fss_out = open(FIFO_OUT, O_RDONLY | O_NONBLOCK);
    if (fss_out == -1) {
        return 1;
    }

    // Initialize hash table.
    table = init_hash_table();

    // Initialize Queue.
    q = init_queue();

    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &block_mask, NULL);
    
    // Read config file
    FILE *fp = fopen(config_path, "r");
    if (!fp) {
        perror("Error opening file.");
        return 1;
    }

    FILE *mlfp = fopen(manager_log_path, "w");
    if (!mlfp) {
        perror("Error opening file.");
        return 1;
    }

    // INITIALIZE INOTIFY INSTANCE. 
    int inotify_fd = inotify_init();

    char line[1024], source_dir[512], target_dir[512];
    
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;

        char *token = strtok(line, " ");
        if(token != NULL) {
            strcpy(source_dir, token);

            token = strtok(NULL,"");
            if(token != NULL) {
                strcpy(target_dir, token);
            } else {
                target_dir[0] = '\0';
                perror("Error with <source> <target> in Config file.");
            }
        }
        // Inserts Valid Directories to hashtable
        if (check_dir(source_dir) == 0 && check_dir(target_dir) == 0) { // NOTE TO SELF TO CHECK IF I NEED TO IMPLEMENT MK DIR IF TARGET DIR DOESWNT EXIST
        
            watchDir* curr = create_dir(source_dir, target_dir);    // Creates directory
            insert_watchDir(table, curr);                           // Inserts to table
            
            curr->active = 1; // set current directory to active (we are watching it). 
            // Start watching directory.
            curr->watchdesc = inotify_add_watch(inotify_fd, source_dir, IN_CREATE | IN_MODIFY | IN_DELETE);
            curr->last_sync_time = time(NULL);


            if (active_workers < worker_count){
                
                spawn_worker(curr->source_dir,curr->target_dir, mlfp, "ALL", "FULL");

            } else { // If active workers > 5
                // Add to queue.
                node* job = init_node(curr->source_dir, curr->target_dir, "ALL", "FULL");
                enqueue(q, job);
            }
        }
    }

    // print_hash_table(table);
    fclose(mlfp);
    fclose(fp); // Close config file

    char buffer[1024];
    struct pollfd fds[2];

    fds[0].fd = inotify_fd;
    fds[0].events = POLLIN;

    fds[1].fd = fss_in;
    fds[1].events = POLLIN;

    sigprocmask(SIG_UNBLOCK, &block_mask, NULL);

    while (1) { 
        poll(fds, 2, -1);

        if (fds[0].revents & POLLIN) {
            ssize_t numRead = read(inotify_fd, buffer, sizeof(buffer));

            for (char* p = buffer; p < buffer + numRead;){
                struct inotify_event* event = (struct inotify_event *) p;
                if (event->len > 0) {
                    watchDir* found = find_watchDir_wd(table, event->wd);
                    if (found == NULL) {
                        p += sizeof(struct inotify_event) + event->len;
                        continue;
                    }
                    char* opt;
                    if (event->mask & IN_CREATE) {
                        printf("IN_CREATE: %s\n", event->name);
                        opt = "ADDED";
                    } else if (event->mask & IN_MODIFY) {
                        printf("IN_MODIFY: %s\n", event->name);
                        opt = "MODIFIED";
                    } else if (event->mask & IN_DELETE) {
                        printf("IN_DELETE: %s\n", event->name);
                        opt = "DELETED";
                    }

                    // We still have to check if there are available workers
                    if (active_workers < worker_count){
                
                        spawn_worker(found->source_dir, found->target_dir, NULL, event->name, opt);
        
                    } else { // If active workers > 5
                        // Add to queue.
                        node* job = init_node(found->source_dir, found->target_dir, event->name, opt);
                        enqueue(q, job);
                    }
                }
                p += sizeof(struct inotify_event) + event->len;
            }
        }

        if (fds[1].revents & POLLIN) {
            ssize_t r = read(fss_in, buffer, sizeof(buffer) - 1);
            if (r > 0) {
         
                char* command = strtok(buffer," ");
                char* source = strtok(NULL," ");
                char* target = strtok(NULL," ");
                
                if (strcmp(command, "add") == 0) {
                    manager_add(source, target, inotify_fd, table, q);
                } else if (strcmp(command, "cancel") == 0) {
                    manager_cancel(source,inotify_fd, table);
                } else if (strcmp(command, "status") == 0) {
                    manager_status(source, table);
                } else if (strcmp(command, "sync") == 0) {
                    manager_sync(source, table, inotify_fd);
                } else {
                    manager_shutdown(table,inotify_fd);
                    break;
                }

            } else if (r == 0) {
                close(fss_in);
                fss_in = open(FIFO_IN, O_RDONLY | O_NONBLOCK);
                fds[1].fd = fss_in;
            }
        }
      
    }

    // Free memory

    destroy_hash_table(table);
    delete_named_pipe(FIFO_IN);
    delete_named_pipe(FIFO_OUT);

    return 0;
}
