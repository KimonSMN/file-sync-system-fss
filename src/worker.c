#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

int copy_file(char* path_from, char* path_to ){
    
    DIR* source = opendir(path_from);
    if (source == NULL) {
        return 1;
    }

    struct dirent* entity;
    entity = readdir(source);
    while (entity != NULL) {
        if(strcmp(entity->d_name, ".") != 0 && strcmp(entity->d_name, "..") != 0 ) {
            char path[100] = { 0 };
            strcat(path, path_from); // pathfrom
            strcat(path, "/"); // pathfrom/
            strcat(path, entity->d_name); // pathfrom/file_name
            int fd = open(path, O_RDONLY);
            if (fd == -1)
                return 1;
            char buffer[1024];
        
            int bytesRead = read(fd, buffer, sizeof(buffer));
            printf("%s\nFile Contents: %s\n",entity->d_name, buffer);
        }
        entity = readdir(source);

    }
    return 0;
}


int main(int argc, char* argv[]){
    printf("Goodmorning, I am worker: %d. I am assigned to watch %s -> %s\n", getpid(), argv[1], argv[2]);
    copy_file(argv[1],"./dummy/docs/a.txt" );
    //open, read, write, unlink, close

    return 0;
}