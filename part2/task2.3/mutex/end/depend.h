#define _GNU_SOURCE

#pragma once

#include <stdio.h>
#include <pthread.h>

#include "storage-mut.h"

int my_pthread_mutex_lock(pthread_mutex_t* mutex);
int my_pthread_mutex_unlock(pthread_mutex_t* mutex);
int my_pthread_mutex_destroy(pthread_mutex_t* mutex);

int show_storage(Storage* s);

int change_increase(int count);
int change_decrease(int count);
int change_equal(int count);
int change_swap();

int init_syncs();
void destroy_syncs();
