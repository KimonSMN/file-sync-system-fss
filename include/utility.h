#ifndef UTILITY
#define UTILITY

#define MAX_WORKERS 5  // MAX IS 5.
#define MANAGER_LOG_PATH "./logs/manager-log"
#define CONSOLE_LOG_PATH "./logs/console-log"
#define CONFIG_PATH "./config.txt"
#define WORKER_PATH "./build/worker"

/* Create a new FIFO (with error check). */
int create_named_pipe(char *name);

/* Write formatted output to stream & to stdout. */
void printf_fprintf(FILE* stream, char* format, ...);

/* If return is 1, path doesn't exist.  
   If return is 0, path exists.  */
int check_dir(const char *path);


#endif