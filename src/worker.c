#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    printf("HELLO I AM WORKER WITH PID: %d AND I RUN\n", getpid());
    return 0;
}