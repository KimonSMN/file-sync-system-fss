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


int create_named_pipe(char *name){
    if(mkfifo(name, 0777) == -1){
        if (errno != EEXIST){
            printf("Could not create fifo file\n");
            return 1;
        }
    }
    return 0;
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


int main(int argc, char* argv[]){

    char* manager_log, *config_file;
    int worker_limit = 5;   // default value is 5.

    // Flags
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            manager_log = argv[++i];
        } else if (strcmp(argv[i], "-c") == 0) {
            config_file = argv[++i];
        } else if (strcmp(argv[i], "-n") == 0){
            worker_limit = atoi(argv[++i]);
        }
    }

    if (manager_log == NULL || config_file == NULL || worker_limit < 5) {
        printf("Usage: ./fss_manager -l <manager_logfile> -c <config_file> -n <worker_limit>\n");
        return 1;
    }

    // Create necessary named-pipes
    create_named_pipe("fss_in");
    create_named_pipe("fss_out");

    // Initialize hash table.
    hashTable* table = init_hash_table();

    // Read config file
    FILE *fp = fopen("./config.txt", "r");
    if (!fp) {
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
        if (check_dir(source_dir) == 0 && check_dir(target_dir) == 0) {
            watchDir* curr = create_dir(source_dir, target_dir);
            insert_watchDir(table, curr);
        
            pid_t pid = fork();

            if (pid == 0) {
                // Child process
                char *args[] = {"./build/worker", source_dir, target_dir, NULL};
                execvp(args[0], args);
        
                // If exec fails
                perror("execvp failed");
            } else if (pid > 0) {
                // Parent process
        
                wait(NULL); // Wait for child to finish
                
                printf("Child process finished\n");
            } else {
                perror("fork failed");
            }

        }
    }

    // print_hash_table(table);

    fclose(fp); // Close config file

    

    // printf("Opening...\n");
    // int fd = open("fss_in", O_WRONLY);
    // printf("Open\n");
    // int x = 97;
    // if(write(fd, &x, sizeof(int)) == -1){
    //     return 2;
    // }
    // printf("Written\n");
    // close(fd);
    // printf("Closed\n");

    destroy_hash_table(table);

    return 0;
}
