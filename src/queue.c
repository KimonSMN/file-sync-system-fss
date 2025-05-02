#include <stdio.h>
#include <string.h>

#include "queue.h"

node* init_node(char* source, char* target, char* filename, char* operation){
    node* job = malloc(sizeof(struct node));
    if (job == NULL) return NULL;
    job->source_dir = strdup(source);
    job->target_dir = strdup(target);
    job->filename = strdup(filename);
    job->operation = strdup(operation);
    job->next = NULL;

    return job;
}

void destroy_node(node* job){
    if(job == NULL) return;

    free(job->source_dir);
    free(job->target_dir);
    free(job->filename);
    free(job->operation);
    free(job);
}

queue* init_queue(){
    queue* q = malloc(sizeof(struct queue));
    if(q == NULL) return NULL;
    q->head = NULL;
    q->tail = NULL;
    return q;
}

int enqueue(queue* q, node* job){
    if (q == NULL) return 1;
    if (job == NULL) return 1;
    
    job->next = NULL;
    if (q->tail != NULL){   // if there is a tail connect the node next to it
        q->tail->next = job;
    }
    q->tail = job;
    if(q->head == NULL){
        q->head = job;
    }
    return 0;
}

node* dequeue(queue* q){
    if(q->head == NULL) return NULL;
    
    node* job = q->head;
    q->head = job->next;
    if(q->head == NULL) {
        q->tail = NULL;
    }
    return job;
}

int isEmpty(queue* q){
    if(q->head == NULL) return 1;
    
    return 0;
}

