#include "uthread.h"

void* thr_func(void *arg) {
    int id = -1, stat;

    if (arg != NULL) {
        id = *(int*)arg;
    }

    printf("[thread %d pid:%d ppid:%d tid:%d]\n", id, getpid(), getppid(), gettid());

    for (int i = 0; i < id; ++i) {
        printf("thread %d is running\n", id);
        stat = uthread_shedule();

        if (stat != EXIT_SUCCESS) {
            fprintf(stderr, "thread %d: uthread_shedule() failed\n", id);
            return NULL;
        }
    }

    int new_id = id * 11;

    char *result = (char*)malloc(16);

    if (result == NULL) {
        fprintf(stderr, "thread %d: malloc() failed\n", id);
        return NULL;
    }

    sprintf(result, "result %d", new_id);

    printf("thread %d finished\n", id);

    return (void*)result;
}

int main(void) {
    uthread_t *my_uthreads[3];
    int args[3] = {3, 5, 7};
    char* results[3];
    int stat;

    printf("[main pid:%d ppid:%d tid:%d]\n", getpid(), getppid(), gettid());

    for (int i = 0; i < 3; ++i) {
        stat = uthread_create(&my_uthreads[i], thr_func, (void*)&args[i]);

        if (stat != TRUE) {
            fprintf(stderr, "main: uthread_create() failed\n");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < 3; ++i) {
        stat = uthread_join(my_uthreads[i], (void**)&results[i]);

        if (stat != TRUE) {
            fprintf(stderr, "main: uthread_join() failed\n");
            return EXIT_FAILURE;
        }

        if (results[i] != NULL) {
            printf("thread %d returned: %s\n", args[i], results[i]);
        }
    }

    printf("main finished\n");

    return EXIT_SUCCESS;
}