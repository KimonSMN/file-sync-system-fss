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

#include "sync_info_mem_store.h"
#include "queue.h"

#define MAX_WORKERS 5  // MAX IS 5.

watchDir* create_dir(char* source_dir, char* target_dir){
    watchDir* dir = malloc(sizeof(watchDir));
    dir->source_dir = strdup(source_dir);
    dir->target_dir = strdup(target_dir);
    dir->active = 0;
    dir->last_sync_time = 0;
    dir->error_count = 0;
    dir->next = NULL;
    return dir;
}

int create_named_pipe(char *name){
    if(mkfifo(name, 0777) == -1){
        if (errno != EEXIST){
            printf("Could not create fifo file\n");
            return 1;
        }
    }
    return 0;
}

void printf_fprintf(FILE* stream, char* format, ...){
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);

    va_start(ap, format);
    vfprintf(stream, format, ap);
    va_end(ap);
}

int check_dir(const char *path) {   // MAY HAVE TO CHANGE THIS, IF WE WANT TO EXIT IF THERE IS A NON EXISTENT DIR
    struct stat st;
    if(stat(path, &st) != 0) {
        // fprintf(stderr,"Directory %s doesn't exist.\n", path);
        return 1;
    } else {
        // fprintf(stdout,"All good with %s.\n", path);
        return 0;
    }
}

// global variables
int active_workers;
queue* q;

void handler(){ // SIGCHLD
    // This signal is sent to a parent process whenever one of its child processes terminates or stops.
        
    int status;
    node* job;
    while((waitpid(-1, &status, WNOHANG)) > 0 ) {
        active_workers--;
        if(isEmpty(q) == 0) {    // if the queue isn't empty.
            job = dequeue(q); // remove the worker from it.
            printf("A WORKER IS READY TO RUN %s -> %s\n", job->source_dir, job->target_dir);
        
            pid_t pid = fork(); // new worker
            if (pid == 0) { // child
                char* args[] = {"./build/worker", job->source_dir, job->target_dir, job->filename, job->operation, NULL};
                execvp(args[0], args);

            } else if(pid > 0) { // parent
                
                active_workers++;
                destroy_node(job);
            }
        }
    }
}


int main(int argc, char* argv[]){

    signal(SIGCHLD, handler);

    char* manager_log, *config_file;
    int worker_count = MAX_WORKERS;   // default value is 5 if not specified.
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

    // Create necessary named-pipes
    create_named_pipe("fss_in");
    create_named_pipe("fss_out");

    // Initialize hash table.
    hashTable* table = init_hash_table();

    // Initialize Queue.
    q = init_queue();

    // Read config file
    FILE *fp = fopen("./config.txt", "r");
    if (!fp) {
        perror("Failed to open file");
        return 1;
    }

    FILE *mlfp = fopen("./logs/manager-log", "w");
    if (!mlfp) {
        perror("Failed to open file");
        return 1;
    }

    char line[1024], source_dir[512], target_dir[512];
    
    while(fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;

        char *token = strtok(line, " ");
        if(token != NULL) {
            strcpy(source_dir, token);

            token = strtok(NULL,"");
            if(token != NULL) {
                strcpy(target_dir, token);
            } else {
                target_dir[0] = '\0';
                perror("Problem in Config file");
            }
        }
        // Inserts Valid Directories to hashtable
        if (check_dir(source_dir) == 0 && check_dir(target_dir) == 0) { // NOTE TO SELF TO CHECK IF I NEED TO IMPLEMENT MK DIR IF TARGET DIR DOESWNT EXIST
        
            watchDir* curr = create_dir(source_dir, target_dir);    // Creates directory
            insert_watchDir(table, curr);                           // Inserts to table
            
            if (active_workers < worker_count){
                
                pid_t pid = fork(); 
        
                if (pid == 0) {
                    // Child process
                    char *args[] = {"./build/worker", source_dir, target_dir, "ALL", "FULL", NULL};
                    execvp(args[0], args);

                    // If exec fails
                    perror("execvp failed");
                } else if (pid > 0) {
                    // Parent process

                    active_workers++;   // Increase worker count.

                    // Print messages
                    time_t t = time(NULL);
                    struct tm tm = *localtime(&t);

                    printf_fprintf(mlfp,"[%d-%02d-%02d %02d:%02d:%02d] Added directory: %s -> %s\n",
                        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                        source_dir, target_dir);
                    printf_fprintf(mlfp,"[%d-%02d-%02d %02d:%02d:%02d] Monitoring started for %s\n", 
                        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                        source_dir);
                    
                    curr->active = 1; // set current directory to active (we are watching it). 

                    // wait(NULL); // Wait for child to finish

                    // printf("Child process finished\n");
                } else {
                    perror("fork failed");
                }
            } else { // If active workers > 5
                
                // Add to queue.
                node* job = init_node(source_dir, target_dir, "ALL", "FULL");
                printf("JOB [%s -> %s] QUEUED\n", source_dir, target_dir);
                enqueue(q, job);


                // have to notify when a process ends

            }
        }
    }

    while (1) { sleep(1); }


    // print_hash_table(table);
    fclose(mlfp);
    fclose(fp); // Close config file

    destroy_hash_table(table);

    return 0;
}
