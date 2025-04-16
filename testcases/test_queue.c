#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

void test_enqueue(void) {
    queue* queue = init_queue();
    enqueue(queue, 5);
    enqueue(queue, 3);

    if(queue->head->value == 5 && queue->head->next->value == 3)
        printf("test_enqueue.c Passed.\n");
    else
        printf("test_enqueue.c Failed.\n");

    dequeue(queue);
    dequeue(queue);
    free(queue);
}

void test_dequeue(void){
    queue* queue = init_queue();
    enqueue(queue, 2);
    dequeue(queue);

    if(queue->head == NULL && queue->tail == NULL)
        printf("test_dequeue.c Passed.\n");
    else
        printf("test_dequeue.c Failed.\n");

    free(queue);
}

int main() {
    test_enqueue();
    test_dequeue();
    return 0;
}
