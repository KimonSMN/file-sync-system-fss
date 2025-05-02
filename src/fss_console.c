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

    int fss_in = open(FIFO_IN, O_WRONLY);
    if(fss_in == -1){
        return 1;
    }
    int fss_out = open(FIFO_OUT, O_RDONLY);
    if(fss_out == -1){
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
        
        if (strcmp(command, "add") == 0) {
            if(source && target) {
                write(fss_in, buffer, strlen(buffer) + 1);
                fprintf(console_log_fd, "[%s] Command add %s -> %s.\n", get_time(), source, target);
                ssize_t bytes_read = read(fss_out, buffer, sizeof(buffer) - 1);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    printf("%s", buffer);
                    fflush(stdout);
                    fprintf(console_log_fd, "%s", buffer);
                    fflush(console_log_fd);

                }
            }
            else
                printf("Usage: add <source> <target>");
        } else if (strcmp(command, "cancel") == 0) {
            if(source && !target) {
                write(fss_in, buffer, strlen(buffer) + 1);
                fprintf(console_log_fd, "[%s] Command cancel %s.\n", get_time(), source);
                ssize_t bytes_read = read(fss_out, buffer, sizeof(buffer) - 1);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';

                    printf("%s", buffer);
                    fflush(stdout);
                    fprintf(console_log_fd, "%s", buffer);
                    fflush(console_log_fd);

                }
            }
            else
                printf("Usage: cancel <source dir>");
        } else if (strcmp(command, "status") == 0) {
            if(source && !target) {
                write(fss_in, buffer, strlen(buffer) + 1);
                fprintf(console_log_fd, "[%s] Command status %s.\n", get_time(), source);
                ssize_t bytes_read = read(fss_out, buffer, sizeof(buffer) - 1);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';

                    printf("%s", buffer);
                    fflush(stdout);
                    fprintf(console_log_fd, "%s", buffer);
                    fflush(console_log_fd);

                }
            }
            else
                printf("Usage: status <source dir>");
        } else if (strcmp(command, "sync") == 0) {
            if(source && !target) {
                write(fss_in, buffer, strlen(buffer) + 1);
                fprintf(console_log_fd, "[%s] Command sync %s.\n", get_time(), source);
                ssize_t bytes_read = read(fss_out, buffer, sizeof(buffer) - 1);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    printf("%s", buffer);
                    fflush(stdout);
                    fprintf(console_log_fd, "%s", buffer);
                    fflush(console_log_fd);

                }
            }
            else    
                printf("Usage: sync <source dir>");
        } else if (strcmp(command, "shutdown") == 0) {
            if(!source) {
                write(fss_in, buffer, strlen(buffer) + 1);
                fprintf(console_log_fd, "[%s] Command shutdown.\n", get_time());
                ssize_t bytes_read;
                while ((bytes_read = read(fss_out, buffer, sizeof(buffer) - 1)) > 0) {
                    buffer[bytes_read] = '\0';
                    printf("%s", buffer);
                    fflush(stdout);
                    fprintf(console_log_fd, "%s", buffer);
                    fflush(console_log_fd);
                }
                active = 0;
                
            }
        } else {
            perror("Unrecognized command");
        }
    }
    
    close(fss_in);
    close(fss_out);

    // fclose(console_log_fd);
    return 0;
}
