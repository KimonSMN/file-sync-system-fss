#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    printf("Goodmorning, I am worker: %d. I am assigned to watch %s -> %s\n", getpid(), argv[1], argv[2]);

    return 0;
}