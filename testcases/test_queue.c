#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

void test_enqueue(void) {
    queue* q = init_queue();

    node* job = init_node("/source", "/target", "test.txt", "FULL");
    node* job1 = init_node("/source1", "/target1", "test1.txt", "FULL");

    enqueue(q, job);
    enqueue(q, job1);

    if (q->head != NULL &&
        strcmp(q->head->source_dir, "/source") == 0 &&
        strcmp(q->head->next->source_dir, "/source1") == 0) {
        printf("test_enqueue Passed.\n");
    } else {
        printf("test_enqueue Failed.\n");
    }

    // Cleanup
    node* j;
    while ((j = dequeue(q)) != NULL) {
        free(j->source_dir);
        free(j->target_dir);
        free(j->filename);
        free(j->operation);
        free(j);
    }
    free(q);
}

void test_dequeue(void) {
    queue* q = init_queue();

    node* job = init_node("/source", "/target", "test.txt", "FULL");

    enqueue(q, job);

    node* removed = dequeue(q);

    if (removed != NULL && strcmp(removed->filename, "test.txt") == 0 && q->head == NULL && q->tail == NULL) {
        printf("test_dequeue Passed.\n");
    } else {
        printf("test_dequeue Failed.\n");
    }

    destroy_node(removed);

    free(q);
}

void test_is_empty(void){
    queue* q = init_queue();

    node* job = init_node("/source", "/target", "test.txt", "FULL");

    enqueue(q, job);

    dequeue(q);

    if (isEmpty(q)) {
        printf("test_is_empty Passed.\n");
    } else {
        printf("test_is_empty Failed.\n");
    }

    destroy_node(job);
    free(q);
}

void test_size(void){
    queue* q = init_queue();

    node* job = init_node("/source", "/target", "test.txt", "FULL");

    enqueue(q, job);
    
    if (sizeOfQueue(q) == 1) {
        printf("test_size Passed.\n");
    } else {
        printf("test_size Failed.\n");
    }

    dequeue(q);

    destroy_node(job);
    free(q);
}

int main() {
    test_enqueue();
    test_dequeue();
    test_is_empty();
    test_size();
    return 0;
}
