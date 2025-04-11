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
        fprintf(stderr,"Directory %s doesn't exist.\n", path);
        return 1;
    } else {
        fprintf(stdout,"All good with %s.\n", path);
        return 0;
    }
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
        check_dir(source_dir);
        check_dir(target_dir);
        // HAVE TO ADD SYNC_INFO_MEM_STORE
    }

    fclose(fp); // Close config file
    printf("PROGRAM PID: %d\n",getpid());

    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        char *args[] = {"./build/worker", "arg1", "arg2", NULL};
        execvp(args[0], args);
        printf("WORKER PID SHOULD BE: %d\n",getpid());

        // If exec fails
        perror("execvp failed");
    } else if (pid > 0) {
        // Parent process

        wait(NULL); // Wait for child to finish
        
        printf("Child process finished\n");
    } else {
        perror("fork failed");
    }
    printf("PROGRAM PID: %d\n",getpid());

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
    return 0;

}
