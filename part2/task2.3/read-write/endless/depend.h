#define _GNU_SOURCE

#pragma once

#include <stdio.h>
#include <pthread.h>

#include "storage-rw.h"

int my_pthread_rwlock_wrlock(pthread_rwlock_t* rwlock);
int my_pthread_rwlock_rdlock(pthread_rwlock_t* rwlock);
int my_pthread_rwlock_unlock(pthread_rwlock_t* rwlock);
int my_pthread_rwlock_destroy(pthread_rwlock_t* rwlock);

int show_storage(Storage* s);

int change_increase(int count);
int change_decrease(int count);
int change_equal(int count);
int change_swap();

int init_syncs();
void destroy_syncs();
