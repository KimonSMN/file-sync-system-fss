#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

int copy_file(char* path_from, char* path_to ){
    
    // Opens Source Directory.
    DIR* source = opendir(path_from);
    if (source == NULL) // If Null return 1.
        return 1;
    
    // Initialie a dirent.
    struct dirent* source_entity;
    source_entity = readdir(source); // Read the Source Directory.
    
    // While there are files in the directory.
    while (source_entity != NULL) {
        if(strcmp(source_entity->d_name, ".") != 0 && strcmp(source_entity->d_name, "..") != 0 ) {  // And these files are not the dir "." & "..".
            // Recreate the path and add the current file.
            char path[100] = { 0 };
            strcat(path, path_from);                // source_directory
            strcat(path, "/");                      // source_directory/
            strcat(path, source_entity->d_name);    // source_directory/file_name
            
            int fd = open(path, O_RDONLY); // Open File.
            if (fd == -1)
                return 1;

            char buffer[4096];
            int bytesRead = read(fd, buffer, sizeof(buffer)); // Read File.
        
            close(fd); // Close File.

            // Opens Target Directory.
            DIR* target = opendir(path_to);
            if (source == NULL) 
                return 1;

            // Recreate the path and add the previous file to the Directory
            char path2[100] = { 0 };
            strcat(path2, path_to);                 // target_directory
            strcat(path2, "/");                     // target_directory/
            strcat(path2, source_entity->d_name);   // target_directory/source_file_name

            int target_fd = open(path2, O_WRONLY | O_CREAT | O_TRUNC, 0777);  // Creates file if it doesnt exist. If it does it overwrites it. 
            if(write(target_fd, buffer, bytesRead) != bytesRead){   // Writes Data to the File.
                perror("Error: Writing Failed\n");
            }   
            close(target_fd);
        }
        source_entity = readdir(source);
    }
    return 0;
}


int main(int argc, char* argv[]){
    printf("Goodmorning, I am worker: %d. I am assigned to watch %s -> %s\n", getpid(), argv[1], argv[2]);
    copy_file(argv[1],"./dummy/backup" );
    //open, read, write, unlink, close

    return 0;
}