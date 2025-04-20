#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char* argv[]){

    int fd = open("./fss_in", O_WRONLY);
    if(fd == -1){
        return 1;
    }
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
            if(source && target) 
                write(fd, buffer, strlen(buffer) + 1);
            else
                printf("Usage: add <source> <target>");
        } else if (strcmp(command, "cancel") == 0) {
            if(source && !target) 
                write(fd, buffer, strlen(buffer) + 1);
            else
                printf("Usage: cancel <source dir>");
        } else if (strcmp(command, "status") == 0) {
            if(source && !target) 
                write(fd, buffer, strlen(buffer) + 1);
            else
                printf("Usage: status <source dir>");
        } else if (strcmp(command, "sync") == 0) {
            if(source && !target) 
                write(fd, buffer, strlen(buffer) + 1);
            else    
                printf("Usage: sync <source dir>");
        } else if (strcmp(command, "shutdown") == 0) {
            if(!source) {
                write(fd, buffer, strlen(buffer) + 1);
                active = 0;
            }
        } else {
            perror("Unrecognized command");
        }

    }

    close(fd);

    return 0;
}
