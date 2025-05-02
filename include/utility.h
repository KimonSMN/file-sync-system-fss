#ifndef UTILITY
#define UTILITY

#include <stdio.h>
#include <time.h>

// MACROS
#define PATH_SIZE 64
#define MAX_WORKERS 5
#define WORKER_PATH "build/worker"
#define FIFO_IN "fss_in"
#define FIFO_OUT "fss_out"
#define BUFFER_SIZE 4096
#define BUFFER_SIZE_SMALL 1024
// Paths have defaults, but will be changed once fss_manager & fss_console by calling set_path() functions.
extern char manager_log_path[PATH_SIZE];
extern char console_log_path[PATH_SIZE];
extern char config_path[PATH_SIZE];

// Function Declerations.

/* Create a new FIFO (with error check). */
int create_named_pipe(const char *name);

/* Delete the given FIFO with error check).*/
int delete_named_pipe(const char *name);

/* Function to set the paths correctly, for use in multiple files. */
void set_path_manager(const char* manager_log, const char* config);

/* Function to set the paths correctly, for use in multiple files. */
void set_path_console(const char* console_log);

/* Write formatted output to stream & to stdout. */
void printf_fprintf(FILE* stream, char* format, ...);

void print_to_buffer(char* sync_report,struct tm tm, char* source, char* target, int worker_pid, char* operation, char* result, char* details);

// void log_to_pipe(int pipe,);

/* If return is 1, path doesn't exist.  
   If return is 0, path exists.  */
int check_dir(const char *path);

/* Return 0 if worker successfuly spawned.
   Return -1 if something went wrong. 
   Set manager_file_pointer to NULL in order not to print a message.*/
int spawn_worker(char* source, char* target, FILE* manager_file_pointer, char* event_name, char* operation);

/* Return the current time. */
char* get_time();

#endif