#include "thread_poll.h"

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//безопасное освобождение мьютекса
int safe_pthread_mutex_unlock(pthread_mutex_t *mutex) {
    int stat = pthread_mutex_unlock(mutex);
    
    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_mutex_unlock failed: %s\n", strerror(stat));
        return stat;
    }
    
    return EXIT_SUCCESS;
}

//логика создание очереди
qhead_t* create_queue() {
    qhead_t *qhead = (qhead_t *)malloc(sizeof(qhead_t));
    
    if (qhead == NULL) {
        fprintf(stderr, "Error: malloc failed for create_queue\n");
        return NULL;
    }
    
    qhead->first = NULL;
    qhead->last = NULL;

    int stat = pthread_mutex_init(&qhead->lock, NULL);
    
    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_mutex_init failed for create_queue: %s\n", strerror(stat));
        free(qhead);
        return NULL;
    }
    
    return qhead;
}

//логика уничтожения очереди
void destroy_queue(qhead_t* qhead) {
    if (qhead == NULL) {
        return;
    }
    
    int stat = pthread_mutex_lock(&qhead->lock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_mutex_lock failed: %s\n", strerror(stat));
        return;
    }
    
    qitem_t *item = qhead->first;
    qitem_t *next = NULL;
    
    while (item != NULL) {
        next = item->next;
        free(item);
        item = next;
    }

    stat = pthread_mutex_unlock(&qhead->lock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_mutex_unlock failed: %s\n", strerror(stat));
        free(qhead);
        return;
    }

    stat = pthread_mutex_destroy(&qhead->lock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_mutex_destroy failed: %s\n", strerror(stat));
        free(qhead);
        return;
    }
    
    free(qhead);
}

//ложика извлечения элемента из очереди
int get_item(qhead_t* qhead) {
    if (qhead == NULL) {
        return NULL_ERROR;
    }
    
    int stat = pthread_mutex_lock(&qhead->lock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_mutex_lock failed: %s\n", strerror(stat));
        return MUTEX_LOCK_ERROR;
    }

    int ret = -1;
    qitem_t *item = NULL;

    if ((qhead->first != NULL) && (qhead->first == qhead->last)) {
        item = qhead->first;
        ret = qhead->first->cli_socket;
        qhead->first = NULL;
        qhead->last = NULL;
    } else if ((qhead->first != NULL) && (qhead->first != qhead->last)) {
        item = qhead->first;
        ret = qhead->first->cli_socket;
        qhead->first = qhead->first->next;
    }
    
    stat = pthread_mutex_unlock(&qhead->lock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_mutex_unlock failed: %s\n", strerror(stat));
        return MUTEX_UNLOCK_ERROR;
    }

    if (item != NULL) {
        free(item);
    }
    
    return ret;
}

//логика добавления элемента в очередь
int add_item(qhead_t* qhead, int fd) {
    if (qhead == NULL) {
        return EXIT_FAILURE;
    }
    
    int stat = pthread_mutex_lock(&qhead->lock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_mutex_lock failed: %s\n", strerror(stat));
        return stat;
    }

    qitem_t *item = (qitem_t *)malloc(sizeof(qitem_t));
    
    if (item == NULL) {
        fprintf(stderr, "Error: malloc failed for item\n");
        safe_pthread_mutex_unlock(&qhead->lock);
        return EXIT_FAILURE;
    }
    
    item->cli_socket = fd;
    item->next = NULL;

    if (qhead->first == NULL) {
        qhead->first = item;
        qhead->last = item;
    } else {
        qhead->last->next = item;
        qhead->last = item;
    }
    
    stat = pthread_mutex_unlock(&qhead->lock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_mutex_unlock failed: %s\n", strerror(stat));
        return stat;
    }
    
    return EXIT_SUCCESS;
}

//логика создания пула потоков
thread_poll_t *thread_poll_create(unsigned int nthreads) {
    thread_poll_t *tp = (thread_poll_t *)malloc(sizeof(thread_poll_t));
    
    if (tp == NULL) {
        fprintf(stderr, "Error: malloc failed for thread_poll_create\n");
        return NULL;
    }
    
    tp->nthreads = nthreads;
    tp->threads = (pthread_t *)malloc(nthreads * sizeof(pthread_t));
    
    if (tp->threads == NULL) {
        fprintf(stderr, "Error: malloc failed for threads\n");
        free(tp);
        return NULL;
    }
    
    tp->queue = create_queue();
    
    if (tp->queue == NULL) {
        fprintf(stderr, "Error: create_queue failed\n");
        free(tp->threads);
        free(tp);
        return NULL;
    }
    
    tp->state = INIT;

    printf("Thread pool with %d created\n", nthreads);
    
    return tp;
}

//логика уничтожения пула потоков
void thread_poll_destroy(thread_poll_t *tp) {
    if (tp == NULL) {
        return;
    }
    
    if (tp->state == RUNNING) {
        int stat;

        for (unsigned int i = 0; i < tp->nthreads; ++i) {
            stat = pthread_join(tp->threads[i], NULL);
            
            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "Error: pthread_join failed: %s\n", strerror(stat));
            }

            printf("Thread %d joined\n", i);
        }
    }
    
    destroy_queue(tp->queue);
    free(tp->threads);
    free(tp);
}

//логика запуска пула потоков
void thread_poll_start(thread_poll_t *tp, void *(*func)(void *), void *args, size_t args_size) {
    if (tp == NULL) {
        return;
    }
    
    if (tp->state == RUNNING) {
        fprintf(stderr, "Error: thread pool is running\n");
        return;
    }
    
    int stat;
    
    for (unsigned int i = 0; i < tp->nthreads; ++i) {
        stat = pthread_create(&(tp->threads[i]), NULL, func, args + i * args_size);
        
        if (stat != EXIT_SUCCESS) {
            fprintf(stderr, "Error: pthread_create failed: %s\n", strerror(stat));
        }
    }

    tp->state = RUNNING;
}