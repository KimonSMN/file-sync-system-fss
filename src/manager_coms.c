#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#include "manager_coms.h"
#include "utility.h"
#include "sync_info_mem_store.h"
#include "queue.h"

/*                           Εντολές που µπορεί να δεχθεί ο fss_manager                         */


// ξεκινά άµεσα τον συγχρονισµό των περιεχοµένων του στον κατάλογο <target>. !!!!!!!!!!!!!! NOT DONE

int manager_add(char* source, char* target, int inotify_fd, hashTable* table, int active_workers, int worker_count, queue* q){
    watchDir* found = find_watchDir(table,source);
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    if (found != NULL) {
        if (strcmp(found->source_dir, source) == 0 && strcmp(found->target_dir, target) == 0) {
            printf("[%d-%02d-%02d %02d:%02d:%02d] Already in queue: %s\n",
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                source);
            return 0;
        }
    }
    
    found = create_dir(source, target);
    insert_watchDir(table, found);
    
    FILE *fp = fopen(MANAGER_LOG_PATH, "a");
    if (!fp) {
        perror("Failed to open file");
        return 1;
    }
    
    if (active_workers < worker_count) {
        pid_t pid = fork();

        if (pid == 0) {
            char *args[] = {WORKER_PATH, source, target, "ALL", "FULL", NULL};
            execvp(args[0], args);

            perror("execvp failed");
        } else if (pid > 0) {
            active_workers++;

            printf_fprintf(fp,"[%d-%02d-%02d %02d:%02d:%02d] Added directory: %s -> %s\n",
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                source, target);
            printf_fprintf(fp,"[%d-%02d-%02d %02d:%02d:%02d] Monitoring started for %s\n", 
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                source);

            found->active = 1;
            found->watchdesc = inotify_add_watch(inotify_fd, source, IN_CREATE | IN_MODIFY | IN_DELETE);
        } else {
            perror("fork failed");
        }
    } else {
        node* job = init_node(source, target, "ALL", "FULL");
        enqueue(q, job);
        printf("JOB [%s -> %s] QUEUED\n", source, target);
    }

    fclose(fp);
    return 0;
}