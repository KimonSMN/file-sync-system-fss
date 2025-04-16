#include <stdio.h>
#include "queue.h"

queue* init_queue(){
    queue* q = malloc(sizeof(struct queue));
    q->head = NULL;
    q->tail = NULL;
    return q;
}

int enqueue(queue* q, int value){
    node* node = malloc(sizeof(struct node)); // create node
    if (node == NULL){
        return 1;
    }
    node->value = value;
    node->next = NULL;
    if (q->tail != NULL){   // if there is a tail connect the node next to it
        q->tail->next = node;
    }
    q->tail = node;
    if(q->head == NULL){
        q->head = node;
    }
    return 0;
}

int dequeue(queue* q){
    if(q->head == NULL){
        return 1;
    }
    node* tmp = q->head;
    int result = tmp->value;
    q->head = q->head->next;
    if(q->head == NULL) {
        q->tail = NULL;
    }
    free(tmp);
    return result;
}

// int isEmpty(queue* q){

// }

// int sizeOfQueue(queue* q){

// }