#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

#include "utility.h"


int copy_file(char* path_from, char* path_to, char* report_buff){
    
    // Opens Source Directory.
    DIR* source = opendir(path_from);
    if (source == NULL) // Sanity check.
        return 1;

    // Initialie a dirent.
    struct dirent* source_entity;
    source_entity = readdir(source); // Read the Source Directory.

    while (source_entity != NULL) {                                                                 // While there are files in the directory.
        if(strcmp(source_entity->d_name, ".") != 0 && strcmp(source_entity->d_name, "..") != 0 ) {  // And these files are not the dir "." & "..".
            // Construct the path of source:
            char path[100] = { 0 };                 // Ex.
            strcat(path, path_from);                // source_directory
            strcat(path, "/");                      // source_directory/
            strcat(path, source_entity->d_name);    // source_directory/file_name
            
            int fd = open(path, O_RDONLY);          // Open `source` dir for reading.
            if (fd == -1) {
                snprintf(report_buff, BUFFER_SIZE, "File %s: %s\n", path, strerror(errno));
                return 1;
            }
        
            // Construct the path of target:
            char path2[100] = { 0 };                // Ex.
            strcat(path2, path_to);                 // target_directory
            strcat(path2, "/");                     // target_directory/
            strcat(path2, source_entity->d_name);   // target_directory/source_file_name

            int target_fd = open(path2, O_WRONLY | O_CREAT | O_TRUNC, 0777);  // Creates file If it doesn't exist. If it does it overwrites it. 
            if (target_fd == -1) {
                snprintf(report_buff, BUFFER_SIZE, "File %s: %s\n", path2, strerror(errno));
                return 1;
            }

            char buffer[4096];
            ssize_t bytesRead;
            while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {    // Read from source file.
                if (write(target_fd, buffer, bytesRead) != bytesRead) {     // Write to target file.
                    snprintf(report_buff, BUFFER_SIZE, "File %s: %s\n", path2, strerror(errno));
                    break;
                }
            }
            if (close(fd)) {
                snprintf(report_buff, BUFFER_SIZE, "File %s: %s\n", path, strerror(errno));
                return 1;
            }
            if (close(target_fd)) {
                snprintf(report_buff, BUFFER_SIZE, "File %s: %s\n", path2, strerror(errno));
                return 1;
            }
        }
        source_entity = readdir(source);
    }
    closedir(source);
    return 0;
}

int add_file(char* source, char* target, char* filename, char* report_buff) {
        
    // Construct the path of source:
    char path[100] = { 0 };
    strcat(path, source);
    strcat(path, "/");
    strcat(path, filename);

    int source_fd = open(path, O_RDONLY); // Open source File.
    if (source_fd == -1) {
        snprintf(report_buff, BUFFER_SIZE, "File %s: %s\n", path, strerror(errno));
        return 1;
    }

    // Construct the path of target:
    char path2[100] = { 0 };
    strcat(path2, target);
    strcat(path2, "/");
    strcat(path2, filename);

    int target_fd = open(path2, O_WRONLY | O_CREAT, 0777); // Open target file.
    if (target_fd == -1) {
        snprintf(report_buff, BUFFER_SIZE, "File %s: %s\n", path2, strerror(errno));
        return 1;
    }

    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = read(source_fd, buffer, sizeof(buffer))) > 0) { // Read the source file.
        if (write(target_fd, buffer, bytesRead) != bytesRead) {         // Write to target file.
            snprintf(report_buff, BUFFER_SIZE, "File %s: %s\n", path2, strerror(errno));
            close(source_fd); 
            close(target_fd);
            break;
        }
    }
    if (close(source_fd) == -1) {
        snprintf(report_buff, BUFFER_SIZE, "File %s: %s\n", path, strerror(errno));
    }
    if (close(target_fd) == -1) {
        snprintf(report_buff, BUFFER_SIZE, "File %s: %s\n", path2, strerror(errno));
    }
    return 0;
}
int modify_file(char* source, char* target, char* filename, char* report_buff) {

    // Construct path of source:
    char path[100] = { 0 };
    strcat(path, source);
    strcat(path, "/");
    strcat(path, filename);

    int source_fd = open(path, O_RDONLY); // Open source File.
    if (source_fd == -1) {
        snprintf(report_buff, BUFFER_SIZE, "File %s: %s\n", path, strerror(errno));
        return 1;
    }

    // Construct path of target:
    char path2[100] = { 0 };
    strcat(path2, target);
    strcat(path2, "/");
    strcat(path2, filename);

    int target_fd = open(path2, O_WRONLY | O_CREAT | O_TRUNC, 0777); // Open target file.
    if (target_fd == -1) {
        snprintf(report_buff, BUFFER_SIZE, "File %s: %s\n", path2, strerror(errno));
        return 1;
    }

    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = read(source_fd, buffer, sizeof(buffer))) > 0) { // Read from source.
        if (write(target_fd, buffer, bytesRead) != bytesRead) {         // Write to target.
            snprintf(report_buff, BUFFER_SIZE, "File %s: %s\n", path2, strerror(errno));
            break;
        }
    }
    close(source_fd); 
    close(target_fd);
    return 0;
}

int delete_file(char* source, char* target, char* filename, char* report_buff) {

    // Construct target path:
    char path2[100] = { 0 };
    strcat(path2, target);
    strcat(path2, "/");
    strcat(path2, filename);

    // Delete the file with unlink.
    if (unlink(path2) != 0) {
        snprintf(report_buff, BUFFER_SIZE,"File %s: %s\n", path2, strerror(errno));
        return 1;
    }
    return 0;
}


int main(int argc, char* argv[]){
    // FULL, ADDED, MODIFIED, DELETED

    char report_buff[4096] = {0};
    // char* sync_report = malloc(BUFFER_SIZE); 
    if(strcmp(argv[3], "ALL") == 0 && strcmp(argv[4], "FULL") == 0){
        copy_file(argv[1], argv[2], report_buff);   // FULL SYNC
        // print_to_buffer(sync_report, tm,argv[1],argv[2],getpid(),argv[4],NULL,NULL);
        // write(STDOUT_FILENO, sync_report, strlen(sync_report));
        // free(sync_report);
        exit(1);
    }

    if(strcmp(argv[4], "ADDED") == 0){
        add_file(argv[1], argv[2], argv[3], report_buff);
        // print_to_buffer(sync_report, tm,argv[1],argv[2],getpid(),argv[4],NULL,NULL);
        // write(STDOUT_FILENO, sync_report, strlen(sync_report));
        // free(sync_report);
        exit(1);
    }

    if(strcmp(argv[4], "MODIFIED") == 0){
        modify_file(argv[1], argv[2], argv[3], report_buff);
        // print_to_buffer(sync_report, tm,argv[1],argv[2],getpid(),argv[4],NULL,NULL);
        // write(STDOUT_FILENO, sync_report, strlen(sync_report));
        // free(sync_report);
        exit(1);
    }

    if(strcmp(argv[4], "DELETED") == 0){
        delete_file(argv[1], argv[2], argv[3], report_buff);
        // print_to_buffer(sync_report, tm,argv[1],argv[2],getpid(),argv[4],NULL,NULL);
        // write(STDOUT_FILENO, sync_report, strlen(sync_report));
        // free(sync_report);
        exit(1);
    }

    // write(STDOUT_FILENO, report_buff, strlen(report_buff));
    return 0;
}