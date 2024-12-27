#include "cache.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

int safe_rwlock_destroy(pthread_rwlock_t *rwlock) {
    int stat = pthread_rwlock_destroy(rwlock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_rwlock_destroy: %s\n", strerror(stat));
    }

    return stat;
}

int safe_rwlock_unlock(pthread_rwlock_t *rwlock) {
    int stat = pthread_rwlock_unlock(rwlock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_rwlock_unlock: %s\n", strerror(stat));
    }

    return stat;
}

int safe_mutex_destroy(pthread_mutex_t *mutex) {
    int stat = pthread_mutex_destroy(mutex);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_mutex_destroy: %s\n", strerror(stat));
    }

    return stat;
}

int safe_cond_destroy(pthread_cond_t *cond) {
    int stat = pthread_cond_destroy(cond);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_cond_destroy: %s\n", strerror(stat));
    }

    return stat;
}

int resource_compare(const void *a, const void *b, void *udata) {
    const resource_t *ra = a;
    const resource_t *rb = b;
    return strcmp(ra->url, rb->url);
}

bool resource_iter(const void *item, void *udata) {
    return true;
}

uint64_t resource_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const resource_t *r = item;
    return hashmap_sip(r->url, strlen(r->url), seed0, seed1);
}

cache_t *cache_create(unsigned int init_size, unsigned int max_size, unsigned int ttl) {
    cache_t *cache = malloc(sizeof(cache_t));
    
    if (cache == NULL) {
        fprintf(stderr, "Error: malloc failed: %s\n", strerror(errno));
        return NULL;
    }

    int stat = pthread_rwlock_init(&cache->rwlock, NULL);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_rwlock_init: %s\n", strerror(stat));
        free(cache);
        return NULL;
    }

    stat = pthread_mutex_init(&cache->mutex, NULL);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_mutex_init: %s\n", strerror(stat));
        safe_rwlock_destroy(&cache->rwlock);
        free(cache);
        return NULL;
    }

    stat = pthread_cond_init(&cache->cond, NULL);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_cond_init: %s\n", strerror(stat));
        safe_rwlock_destroy(&cache->rwlock);
        safe_mutex_destroy(&cache->mutex);
        free(cache);
        return NULL;
    }

    cache->current_size = init_size;
    cache->remaining = init_size;
    cache->max_size = max_size;
    cache->cache_ttl = ttl;
    cache->position = 0;

    errno = EXIT_SUCCESS;

    cache->cache_page = (char*)malloc(init_size * sizeof(char));

    if (cache->cache_page == NULL) {
        fprintf(stderr, "Error: malloc failed: %s\n", strerror(errno));
        safe_rwlock_destroy(&cache->rwlock);
        safe_mutex_destroy(&cache->mutex);
        safe_cond_destroy(&cache->cond);
        free(cache);
        return NULL;
    }

    cache->cache_map = hashmap_new(sizeof(resource_t), 0, 0, 0, resource_hash, resource_compare, NULL, NULL);

    printf("Cache created:\n\tinit_size: %d\n\tmax_size: %d\n\tttl: %d\n", init_size, max_size, ttl);

    return cache;
}

resource_t *find_resource(cache_t *cache, char *url) {
    int stat = pthread_rwlock_rdlock(&cache->rwlock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_rwlock_rdlock: %s\n", strerror(stat));
        return NULL;
    }

    resource_t *new_resource = NULL;
    resource_t *resource = (resource_t*)hashmap_get(cache->cache_map, &(resource_t){.url = url});

    if (resource == NULL) {
        safe_rwlock_unlock(&cache->rwlock);
        return NULL;
    }

    if ((resource->state == DONE) && (time(NULL) - resource->load >= cache->cache_ttl)) {
        safe_rwlock_unlock(&cache->rwlock);
        delete_resource(cache, resource);
        return NULL;
    } else if ((resource->state == DONE) && (time(NULL) - resource->load < cache->cache_ttl)) {
        new_resource = (resource_t*)malloc(sizeof(resource_t));

        if (new_resource == NULL) {
            fprintf(stderr, "Error: malloc failed: %s\n", strerror(errno));
            safe_rwlock_unlock(&cache->rwlock);
            return NULL;
        }

        char* data = (char*)malloc(resource->size * sizeof(char));

        if (data == NULL) {
            fprintf(stderr, "Error: malloc failed: %s\n", strerror(errno));
            free(new_resource);
            safe_rwlock_unlock(&cache->rwlock);
            return NULL;
        }

        memcpy(data, &cache->cache_page[resource->data_position], resource->size * sizeof(char));

        new_resource->data = data;
        new_resource->size = resource->size;
        new_resource->url = resource->url;
        new_resource->state = resource->state;
    } else if (resource->state == IN_PROGRESS) {
        new_resource = (resource_t*)malloc(sizeof(resource_t));
        new_resource->state = IN_PROGRESS;
    } else {
        new_resource = NULL;
    }

    stat = pthread_rwlock_unlock(&cache->rwlock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_rwlock_unlock: %s\n", strerror(stat));
        return NULL;
    }

    return new_resource;
}

void put(cache_t *cache, resource_t *resource) {
    memcpy(&cache->cache_page[cache->position], resource->data, resource->size * sizeof(char));

    resource->data = NULL;
    resource->data_position = cache->position;
    resource->load = time(NULL);
    cache->position += resource->size;
    cache->remaining -= resource->size;

    hashmap_set(cache->cache_map, &(resource_t){.url = resource->url,
                                               .size = resource->size,
                                               .data_position = resource->data_position,
                                               .load = resource->load,
                                               .state = resource->state});

    printf("Put resource %s in cache\n", resource->url);
    printf("Resource size: %d, cache remaining: %d\n", resource->size, cache->remaining);

    int stat = pthread_cond_broadcast(&cache->cond);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_cond_broadcast: %s\n", strerror(stat));
    }
}

void _delete_resource(cache_t *cache, resource_t *resource) {
    printf("Delete resource %s from cache\n", resource->url);

    if (resource->size != 0) {
        memcpy(&cache->cache_page[resource->data_position],
        &cache->cache_page[resource->data_position + resource->size],
        (cache->current_size - resource->size - resource->data_position) * sizeof(char));
        cache->position -= resource->size;
        cache->remaining += resource->size;
    }

    hashmap_set(cache->cache_map, &(resource_t){.url = resource->url, .state = DELETE, .size = 0, .data_position = -1});

    int stat = pthread_cond_broadcast(&cache->cond);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_cond_broadcast: %s\n", strerror(stat));
    }
}

void delete_resource(cache_t *cache, resource_t *resource) {
    int stat = pthread_rwlock_wrlock(&cache->rwlock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_rwlock_wrlock: %s\n", strerror(stat));
        return;
    }

    _delete_resource(cache, resource);

    stat = pthread_rwlock_unlock(&cache->rwlock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_rwlock_unlock: %s\n", strerror(stat));
    }
}


void clear_cache(cache_t *cache) {
    size_t i = 0;
    resource_t *resource;

    while (hashmap_iter(cache->cache_map, &i, (void**)&resource)) {
        if ((time(NULL) - resource->load >= cache->cache_ttl) && (resource->data_position != -1)) {
            _delete_resource(cache, resource);
        }
    }
}

void put_resource(cache_t *cache, resource_t *resource) {
    int stat = pthread_rwlock_wrlock(&cache->rwlock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_rwlock_wrlock: %s\n", strerror(stat));
        return;
    }

    if (resource->size > cache->max_size) {
        safe_rwlock_unlock(&cache->rwlock);
        return;
    }

    if (resource->size <= cache->remaining) {
        printf("Can put resource %s in cache\n", resource->url);
        put(cache, resource);
    } else if ((resource->size > cache->remaining) && (resource->size < cache->max_size)) {
        errno = EXIT_SUCCESS;

        cache->cache_page = (char*)realloc(cache->cache_page, cache->max_size * sizeof(char));
        cache->remaining += cache->max_size - cache->current_size;
        cache->current_size = cache->max_size;

        printf("Cache resized:\n\tcurrent_size: %d\n\tremaining: %d\n", cache->current_size, cache->remaining);

        while (cache->remaining < resource->size) {
            clear_cache(cache);
        }

        put(cache, resource);
    } else {
        printf("clear cache that pur resource, remainig %d, size %d\n", cache->remaining, resource->size);

        while (cache->remaining < resource->size){
            clear_cache(cache);
        }

        put(cache, resource);
    }

    stat = pthread_rwlock_unlock(&cache->rwlock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_rwlock_unlock: %s\n", strerror(stat));
    }
}

void cache_destroy(cache_t *cache) {
    int stat = pthread_rwlock_wrlock(&cache->rwlock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_rwlock_wrlock: %s\n", strerror(stat));
        return;
    }

    size_t i = 0;
    resource_t *resource;

    while (hashmap_iter(cache->cache_map, &i, (void**)&resource)) {
        printf("Free %s\n", resource->url);
        free(resource->url);
    }

    hashmap_free(cache->cache_map);

    stat = pthread_rwlock_unlock(&cache->rwlock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_rwlock_unlock: %s\n", strerror(stat));
    }

    safe_rwlock_destroy(&cache->rwlock);
    safe_mutex_destroy(&cache->mutex);
    safe_cond_destroy(&cache->cond);
    
    free(cache->cache_page);
    free(cache);
}

void set_resource_in_progress(cache_t *cache, char *url) {
    int stat = pthread_rwlock_wrlock(&cache->rwlock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_rwlock_wrlock: %s\n", strerror(stat));
        return;
    }

    resource_t *resource = find_resource(cache, url);

    if ((resource != NULL) && (resource->state == DONE)) {
        delete_resource(cache, &(resource_t){.url = url});
    }
    
    hashmap_set(cache->cache_map, &(resource_t){.url = url, .state = IN_PROGRESS, .size = 0, .data_position = -1});

    stat = pthread_rwlock_unlock(&cache->rwlock);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "Error: pthread_rwlock_unlock: %s\n", strerror(stat));
    }
}