
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "globals.h"
#include "utility.h"

char manager_log_path[PATH_SIZE] = "./logs/manager-log";
char console_log_path[PATH_SIZE] = "./logs/console-log";
char config_path[PATH_SIZE] = "./config.txt";

int create_named_pipe(const char *name){
    if(mkfifo(name, 0777) == -1){
        if (errno != EEXIST){
            printf("Could not create fifo\n");
            return 1;
        }
    }
    return 0;
}
int delete_named_pipe(const char *name){
    if(unlink(name) == -1){
        printf("Could not delete fifo\n");
        return 1;
    }
    return 0;
}

void set_path_manager(const char* manager_log, const char* config){
    if (manager_log != NULL && config != NULL) {
        strncpy(manager_log_path, manager_log, PATH_SIZE);
        strncpy(config_path, config, PATH_SIZE);
    }
}

void set_path_console(const char* console_log){
    if (console_log != NULL) {
        strncpy(console_log_path,console_log, PATH_SIZE);
    }
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

    // Open a new pipe for each worker
    // int fd[2]; // fd[0] - read | fd[1] - write
    // if (pipe(fd) == -1) {
    //     exit(1);
    // }

    pid_t pid = fork(); 
        
    if (pid == 0) {
        // Child process

        // close(fd[0]);   // Child doesn't read.
        // dup2(fd[1], STDOUT_FILENO);  // write end
        // close(fd[1]);

        // race condition here , should be fixed

        char *args[] = {WORKER_PATH, source, target, event_name, operation, NULL};
        execvp(args[0], args);

        perror("Error execvp Failed.");
        exit(1);
    } else if (pid > 0) {
        // Parent process

        // close(fd[1]); // Parent doesn't write.
        
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