#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include "utility.h"

int main(int argc, char* argv[]){

    char* console_log = NULL;

    // Flags
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            console_log = argv[++i];
        }
    }
    if (console_log == NULL) {
        printf("Usage: ./fss_console -l <console-logfile>\n");
        exit(1);
    }

    set_path_console(console_log);

    int fd = open(FIFO_IN, O_WRONLY);
    if(fd == -1){
        return 1;
    }

    FILE* console_log_fd = fopen(console_log, "w");

    char buffer[128];

    int active = 1;

    while (active) {
        printf("\n$ ");

        fgets(buffer, sizeof(buffer), stdin);
        buffer[strlen(buffer)-1] = '\0';

        char tmp[128];
        strncpy(tmp, buffer, sizeof(buffer)); // safer

        char* command = strtok(tmp, " ");
        char* source = strtok(NULL, " ");
        char* target = strtok(NULL, " ");
        
        struct tm tm = get_time();

        if (strcmp(command, "add") == 0) {
            if(source && target) {
                write(fd, buffer, strlen(buffer) + 1);
                fprintf(console_log_fd, "[%d-%02d-%02d %02d:%02d:%02d] Command add %s -> %s.\n",
                    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                    tm.tm_hour, tm.tm_min, tm.tm_sec,
                    source, target);
            }
            else
                printf("Usage: add <source> <target>");
        } else if (strcmp(command, "cancel") == 0) {
            if(source && !target) {
                write(fd, buffer, strlen(buffer) + 1);
                fprintf(console_log_fd, "[%d-%02d-%02d %02d:%02d:%02d] Command cancel %s.\n",
                    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                    tm.tm_hour, tm.tm_min, tm.tm_sec,
                    source);
            }
            else
                printf("Usage: cancel <source dir>");
        } else if (strcmp(command, "status") == 0) {
            if(source && !target) {
                write(fd, buffer, strlen(buffer) + 1);
                fprintf(console_log_fd, "[%d-%02d-%02d %02d:%02d:%02d] Command status %s.\n",
                    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                    tm.tm_hour, tm.tm_min, tm.tm_sec,
                    source);
            }
            else
                printf("Usage: status <source dir>");
        } else if (strcmp(command, "sync") == 0) {
            if(source && !target) {
                write(fd, buffer, strlen(buffer) + 1);
                fprintf(console_log_fd, "[%d-%02d-%02d %02d:%02d:%02d] Command sync %s.\n",
                    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                    tm.tm_hour, tm.tm_min, tm.tm_sec,
                    source);
            }
            else    
                printf("Usage: sync <source dir>");
        } else if (strcmp(command, "shutdown") == 0) {
            if(!source) {
                write(fd, buffer, strlen(buffer) + 1);
                fprintf(console_log_fd, "[%d-%02d-%02d %02d:%02d:%02d] Command shutdown %s.\n",
                    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                    tm.tm_hour, tm.tm_min, tm.tm_sec,
                    source);
                active = 0;
            }
        } else {
            perror("Unrecognized command");
        }
    }
    
    close(fd);
    // fclose(console_log_fd);
    return 0;
}
