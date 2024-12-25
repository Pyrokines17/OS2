#include "../hashmap/hashmap.h"
#include <pthread.h>
#include <time.h>

#define DONE 1
#define IN_PROGRESS 4
#define DELETE -1

typedef struct resource_t {
    char *url;
    unsigned int size;
    char *data;
    int data_position;
    time_t load;
    int state;
} resource_t;

typedef struct cache_t {
    unsigned int cache_ttl;
    unsigned int current_size;
    unsigned int remaining;
    unsigned int position;
    unsigned int max_size;
    char *cache_page;
    struct hashmap *cache_map;
    pthread_rwlock_t rwlock;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} cache_t;

cache_t *cache_create(unsigned int init_size, unsigned int max_size, unsigned int ttl);
resource_t *find_resource(cache_t *cache, char *url);
void put_resource(cache_t *cache, resource_t *resource);
void cache_destroy(cache_t *cache);
void delete_resource(cache_t *cache, resource_t *resource);
void set_resource_in_progress(cache_t *cache, char *url);
