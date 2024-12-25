#include "uthread.h"

uthread_t *uthreads[MAX_THREADS];
int uthreads_ends[MAX_THREADS];

uthread_t main_thread;
ucontext_t *main_cntx;

int uthread_count;
int uthread_curr;

int main_point;

int is_valid_thread(int uthread_id) {
    if (uthreads[uthread_id] == NULL || uthreads_ends[uthread_id] == TRUE) {
        return FALSE;
    } 
    
    return TRUE;
}

int add_main_thread(ucontext_t *main_ucon, uthread_t *main_uthr) {
    if (main_ucon == NULL || main_uthr == NULL) {
        fprintf(stderr, "add_main_thread() : main_ucon or main_uthr is NULL\n");
        return NULL_ERROR;
    }

    main_thread.ucntx.uc_stack.ss_sp = &main_point;
    main_thread.ucntx.uc_stack.ss_size = STACK_SIZE;
    main_thread.ucntx.uc_link = NULL;

    main_thread.tid = 0;
    main_thread.thread_status = RUNNING;

    main_cntx = main_ucon;

    uthreads[0] = main_uthr;
    uthreads_ends[0] = FALSE;

    uthread_count++;

    return EXIT_SUCCESS;
}

void *create_stack(off_t stack_size) {
    void *stack = mmap(NULL, stack_size, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (stack == MAP_FAILED) {
        fprintf(stderr, "create_stack() : mmap() failed: %s\n", strerror(errno));
        return NULL;
    }

    memset(stack, 0x00, stack_size);

    int err = mprotect(stack + PAGE_SIZE, STACK_SIZE - 2 * PAGE_SIZE, PROT_READ | PROT_WRITE);

    if (err == EXIT_FAILURE) {
        fprintf(stderr, "create_stack() : mprotect() failed: %s\n", strerror(err));
        return NULL;
    }

    return stack;
}

int uthread_shedule(void) {
    ucontext_t *curr_cntx, *next_cntx;    
    int old_curr;

    curr_cntx = &(uthreads[uthread_curr]->ucntx);
    old_curr = uthread_curr;

    while (ENDLESS) {
        uthread_curr = (uthread_curr + 1) % uthread_count;

        if (is_valid_thread(uthread_curr) == TRUE) {
            break;
        }

        if (uthread_curr == old_curr) {
            fprintf(stderr, "uthread_shedule() : no valid threads\n");
            return NO_VALID_ERROR;
        }
    }
    
    next_cntx = &(uthreads[uthread_curr]->ucntx);

    int err = swapcontext(curr_cntx, next_cntx);

    if (err == EXIT_FAILURE) {
        fprintf(stderr, "uthread_shedule() : swapcontext() failed: %s\n", strerror(err));
        return SHEDULE_ERROR;
    }

    sleep(1);

    return EXIT_SUCCESS;
}

void starter(uthread_t *uthread) {
    uthread->thread_status = RUNNING;
    uthread->retval = uthread->start_routine(uthread->arg);
    uthread->thread_status = FINISHED;

    int stat;

    while (uthread->thread_status != JOINED) {
        stat = uthread_shedule();

        if (stat != EXIT_SUCCESS) {
            fprintf(stderr, "starter() : uthread_shedule() failed: %s\n", strerror(stat));
            return;
        }
    }
}

int uthread_create(uthread_t **uthread, void *(*start_routine)(void *), void *arg) {
    if (start_routine == NULL) {
        fprintf(stderr, "uthread_create() : start_routine is NULL\n");
        return FALSE;
    }

    if (uthread_count == MAX_THREADS) {
        fprintf(stderr, "uthread_create() : maximum number of threads reached\n");
        return FALSE;
    }

    if (uthread_count == EMPTY) {
        int stat = add_main_thread(&main_thread.ucntx, &main_thread);

        if (stat != EXIT_SUCCESS) {
            return FALSE;
        }
    }

    static int uthread_id = 1;
    uthread_t *uthread_new;

    void *stack = create_stack(STACK_SIZE);

    if (stack == NULL) {
        fprintf(stderr, "uthread_create() : create_stack() failed\n");
        return FALSE;
    }

    uthread_new = (uthread_t*)(stack + STACK_SIZE - sizeof(uthread_t));

    int err = getcontext(&uthread_new->ucntx);

    if (err == EXIT_FAILURE) {
        fprintf(stderr, "uthread_create() : getcontext() failed: %s\n", strerror(err));
        return FALSE;
    }

    uthread_new->ucntx.uc_stack.ss_sp = stack;
    uthread_new->ucntx.uc_stack.ss_size = STACK_SIZE - sizeof(uthread_t);
    uthread_new->ucntx.uc_link = main_cntx;

    uthread_new->start_routine = start_routine;
    uthread_new->arg = arg;
    uthread_new->thread_status = READY;
    
    uthread_new->tid = uthread_id;
    uthread_id++;

    uthreads[uthread_count++] = uthread_new;
    uthreads_ends[uthread_count] = FALSE;
    *uthread = uthread_new;

    makecontext(&uthread_new->ucntx, (void (*)(void))starter, 1, uthread_new);

    return TRUE;
}

int uthread_join(uthread_t *uthread, void **retval) {
    if (uthread == NULL) {
        fprintf(stderr, "uthread_join() : uthread is NULL\n");
        return FALSE;
    }

    int stat;

    while (uthread->thread_status != FINISHED) {
        stat = uthread_shedule();

        if (stat != EXIT_SUCCESS) {
            fprintf(stderr, "uthread_join() : uthread_shedule() failed: %s\n", strerror(stat));
            return FALSE;
        }
    }

    if (retval != NULL) {
        *retval = uthread->retval;
    }

    uthreads_ends[uthread->tid] = TRUE;

    if (uthread->ucntx.uc_stack.ss_sp != NULL) {
        stat = munmap(uthread->ucntx.uc_stack.ss_sp, uthread->ucntx.uc_stack.ss_size);

        if (stat == EXIT_FAILURE) {
            fprintf(stderr, "uthread_join() : munmap() failed: %s\n", strerror(stat));
            return FALSE;
        }
    }

    return TRUE;
}
