#ifndef QUEUE
#define QUEUE

#include <time.h>
#include <stdlib.h>

typedef struct node{

    int value;
    struct node* next;

} node;


typedef struct queue{
    
    node* head;
    node* tail;

} queue;


queue* init_queue();

int enqueue(queue* q, int value);

int dequeue(queue* q);

// int isEmpty(queue* q);

// int sizeOfQueue(queue* q);
#endif