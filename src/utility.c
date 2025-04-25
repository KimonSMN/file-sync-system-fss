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

#include "globals.h"
#include "utility.h"

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
    if(stat(path, &st) != 0) 
        return 1;
    
    return 0;
}


int spawn_worker(char* source, char* target, FILE* manager_file_pointer, char* event_name, char* operation){

    pid_t pid = fork(); 
        
    if (pid == 0) {
        // Child process
        char *args[] = {WORKER_PATH, source, target, event_name, operation, NULL};
        execvp(args[0], args);

        perror("Error execvp Failed.");
        exit(1);
    } else if (pid > 0) {
        // Parent process

        active_workers++;   // Increase worker count.

        if (manager_file_pointer != NULL) {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            
            printf_fprintf(manager_file_pointer,"[%d-%02d-%02d %02d:%02d:%02d] Added directory: %s -> %s\n",
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                source, target);
            printf_fprintf(manager_file_pointer,"[%d-%02d-%02d %02d:%02d:%02d] Monitoring started for %s\n", 
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                source);
        }
 
    } else {
        perror("Error fork Failed.");
        exit(1);
    }
    return 0;
}

struct tm get_time(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    return tm;
}