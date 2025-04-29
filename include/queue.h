#ifndef QUEUE
#define QUEUE

#include <time.h>
#include <stdlib.h>

typedef struct node{
    char* source_dir;
    char* target_dir;
    char* filename;
    char* operation;
    struct node* next;
} node;


typedef struct queue{
    node* head;
    node* tail;
} queue;


node* init_node(char* source, char* target, char* filename, char* operation);

void destroy_node (node* job);

queue* init_queue();

int enqueue(queue* q, node* job);

node* dequeue(queue* q);

int isEmpty(queue* q);

int sizeOfQueue(queue* q);
#endif