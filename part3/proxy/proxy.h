#include "../thread_poll/thread_poll.h"
#include "../cache/cache.h"
#include <pthread.h>

#define BUSY 1
#define FREE 0

#define DEF_THREADS 4
#define DEF_CACHE_INIT_SIZE 1024*1024
#define DEF_CACHE_MAX_SIZE 400*1024*1024
#define DEF_PORT 9000
#define DEF_CACHE_TTL 20

#define BUFFER_SIZE 4096 * 20
#define AMOUNT_EVENTS 1024

#define WAIT_RESPONSE_S 10
#define WAIT_RESPONSE_NS 0

#define TRUE 1
#define FALSE 0

#define REDIRECT -2
#define BAD_GATEWAY -3
#define TIMEOUT -4

//структура прокси сервера
typedef struct proxy_t {
    thread_poll_t *thread_poll;
    int server_socket;
    cache_t *cache;
} proxy_t;

proxy_t *proxy_create();
void proxy_destroy(proxy_t *proxy);
void proxy_start(proxy_t *proxy);
