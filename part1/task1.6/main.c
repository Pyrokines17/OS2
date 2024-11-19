#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "mythread.h"

void* test_func(void* arg){
    char* res = (char*)malloc(sizeof(char)*BUFFER_SIZE);
    int* name = (int*)arg;

    printf("Hello from thread %d\n", *name);

    for (int i = 0; i < 5; ++i) {
        printf("%d\n", i);
    }

    strcpy(res, "Hello");

    return (void*)res;
}

void freeMemory(int border, char* ress[THR_ITER]) {
    for (int i = 0; i < border; ++i) {
        free(ress[i]);
    }
}

int main(void) { 
    mythread_t ts[THR_ITER];
    char* ress[THR_ITER];
    int res;

    for (int i = 0; i < THR_ITER; ++i) {
        res = mythread_create(&ts[i], test_func, (void*)&i);

        if (res != TRUE) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < THR_ITER; ++i) {
        res = mythread_join(ts[i], (void**)&ress[i]);

        if (res != EXIT_SUCCESS) {
            fprintf(stderr, "Error joining thread %d: %s\n", i, strerror(res));
            freeMemory(i, ress);
            return EXIT_FAILURE;
        }

        printf("Thread %d returned: %s\n", i, ress[i]);
    }

    freeMemory(THR_ITER, ress);

    return 0;
}