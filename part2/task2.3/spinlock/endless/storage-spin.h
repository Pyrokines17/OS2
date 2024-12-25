#define _GNU_SOURCE

#pragma once

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#define LOCK_ERROR -6
#define DESTROY_ERROR -5
#define INIT_ERROR -4
#define MALLOC_ERROR -3
#define SYNCS_ERROR -2
#define MAIN_ERROR -1

#define ENDLESS 1
#define EMPTY 0

#define FALSE 0
#define TRUE 1

#define PCOUNT 10

typedef struct _Link {
	int len;
	int count;
} Link;

typedef struct _Node {
	int len;
	char value[100];
	struct _Node* next;
	pthread_spinlock_t sync;
} Node;

typedef struct _Storage {
	pthread_spinlock_t sync;

	Node* first;
	Node* last;

	int count;
	int max_count;
} Storage;

Storage* storage_init(int max_count);
int storage_destroy(Storage* q);
int storage_add(Storage* q, char* val);
int storage_get(Storage* q, char** val);
int fill_storage(Storage* q);
