#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define INIT 0
#define RUNNING 1

#define NULL_ERROR -1
#define MUTEX_LOCK_ERROR -2
#define MUTEX_UNLOCK_ERROR -3

//структура элемента очереди из сокетов
typedef struct qitem_t {
    int cli_socket;
    struct qitem_t *next;
} qitem_t;

//структура очереди
typedef struct qhead_t {
    qitem_t *first;
    qitem_t *last;
    pthread_mutex_t lock;
} qhead_t;

//структура пула потоков
typedef struct thread_poll_t {
    unsigned int nthreads;
    pthread_t *threads;
    qhead_t *queue;
    int state;
} thread_poll_t;

thread_poll_t *thread_poll_create(unsigned int nthreads);
void thread_poll_destroy(thread_poll_t *tp);
void thread_poll_start(thread_poll_t *tp, void *(*func)(void *), void *args, size_t args_size);
int get_item(qhead_t* qhead);
int add_item(qhead_t* qhead, int fd);
