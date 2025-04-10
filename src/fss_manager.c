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


int create_named_pipe(char *name){
    if(mkfifo(name, 0777) == -1){
        if (errno != EEXIST){
            printf("Could not create fifo file\n");
            return 1;
        }
    }
    return 0;
}

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

    // Create necessary named-pipes
    create_named_pipe("fss_in");
    create_named_pipe("fss_out");

    // Read config file

    FILE *fp = fopen("./config.txt", "r");
    if (!fp) {
        perror("Failed to open file");
        return 1;
    }

    char line[1024], source_dir[512], target_dir[512];

    if(fgets(line, sizeof(line), fp)) {
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
    }
    printf("%s ",source_dir);
    printf("%s\n",target_dir);

    fclose(fp);

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
