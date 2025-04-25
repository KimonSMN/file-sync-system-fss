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


            // Recreate the path and add the previous file to the Directory
            char path2[100] = { 0 };
            strcat(path2, path_to);                 // target_directory
            strcat(path2, "/");                     // target_directory/
            strcat(path2, source_entity->d_name);   // target_directory/source_file_name

            int target_fd = open(path2, O_WRONLY | O_CREAT | O_TRUNC, 0777);  // Creates file if it doesnt exist. If it does it overwrites it. 
            
            char buffer[4096];
            ssize_t bytesRead;
            while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
                if (write(target_fd, buffer, bytesRead) != bytesRead) {
                    perror("Copy failed.");
                    break;
                }
            }
            close(fd); // Close File.
            close(target_fd);
        }
        source_entity = readdir(source);
    }
    closedir(source);
    return 0;
}


int add_file(char* source, char* target, char* filename) {
        char path[100] = { 0 };
        strcat(path, source);
        strcat(path, "/");
        strcat(path, filename);

        int source_fd = open(path, O_RDONLY); // Open source File.
        if (source_fd == -1) {
            return 1;
        }

        char path2[100] = { 0 };
        strcat(path2, target);
        strcat(path2, "/");
        strcat(path2, filename);
  
        int target_fd = open(path2, O_WRONLY | O_CREAT, 0777); // Open target file.
        if (target_fd == -1) {
            return 1;
        }
        
        char buffer[4096];
        ssize_t bytesRead;
        while ((bytesRead = read(source_fd, buffer, sizeof(buffer))) > 0) {
            if (write(target_fd, buffer, bytesRead) != bytesRead) {
                perror("Write failed.");
                close(source_fd); 
                close(target_fd);
                break;
            }
        }
        close(source_fd); 
        close(target_fd);
        return 0;
}
int modify_file(char* source, char* target, char* filename) {

    char path[100] = { 0 };
    strcat(path, source);
    strcat(path, "/");
    strcat(path, filename);

    int source_fd = open(path, O_RDONLY); // Open source File.
    if (source_fd == -1)
        return 1;

    char path2[100] = { 0 };
    strcat(path2, target);
    strcat(path2, "/");
    strcat(path2, filename);

    int target_fd = open(path2, O_WRONLY | O_CREAT | O_TRUNC, 0777); // Open target file.
    if (target_fd == -1)
        return 1;

    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = read(source_fd, buffer, sizeof(buffer))) > 0) {
        if (write(target_fd, buffer, bytesRead) != bytesRead) {
            perror("Modify failed.");
            break;
        }
    }
    close(source_fd); 
    close(target_fd);
    return 0;
}

int delete_file(char* source, char* target, char* filename) {
    char path2[100] = { 0 };
    strcat(path2, target);
    strcat(path2, "/");
    strcat(path2, filename);

    if (unlink(path2) != 0) {
        perror("Delete failed.");
        return 1;
    }
    return 0;
}


int main(int argc, char* argv[]){
    // FULL, ADDED, MODIFIED, DELETED

    if(strcmp(argv[3], "ALL") == 0 && strcmp(argv[4], "FULL") == 0){
        sleep(2);
        copy_file(argv[1], argv[2]);   // FULL SYNC
    }

    if(strcmp(argv[4], "ADDED") == 0){
        sleep(2);
        add_file(argv[1], argv[2], argv[3]);
    }

    if(strcmp(argv[4], "MODIFIED") == 0){
        sleep(2);
        modify_file(argv[1], argv[2], argv[3]);
    }

    if(strcmp(argv[4], "DELETED") == 0){
        sleep(2);
        delete_file(argv[1], argv[2], argv[3]);
    }

    return 0;
}