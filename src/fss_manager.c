#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

int main(int argc, char* argv[]){


    // Flags
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            char* manager_log = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0) {
            char* config_file = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0){
            int worker_limit = atoi(argv[++i]);
        }
    }

    if(mkfifo("fss_in", 0777) == -1 ){ // This file read from or wrote to by everyone
        if (errno != EEXIST){
            printf("Could not create fifo file\n");
            return 1;
        }
    }
    if(mkfifo("fss_out", 0777) == -1 ){ // This file read from or wrote to by everyone
        if (errno != EEXIST){
            printf("Could not create fifo file\n");
            return 1;
        }
    }
    printf("Opening...\n");
    int fd = open("fss_in", O_WRONLY);
    printf("Open\n");
    int x = 97;
    if(write(fd, &x, sizeof(int)) == -1){
        return 2;
    }
    printf("Written\n");
    close(fd);
    printf("Closed\n");
    return 0;

}
