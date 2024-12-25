#define _GNU_SOURCE

#pragma once

#include <stdio.h>
#include <pthread.h>

#include "storage-spin.h"

int my_pthread_spin_lock(pthread_spinlock_t* spinlock);
int my_pthread_spin_unlock(pthread_spinlock_t* spinlock);
int my_pthread_spin_destroy(pthread_spinlock_t* spinlock);

int show_storage(Storage* s);

int change_increase(int count);
int change_decrease(int count);
int change_equal(int count);
int change_swap();

int init_syncs();
void destroy_syncs();
