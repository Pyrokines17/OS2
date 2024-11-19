#ifndef __FITOS_QUEUE_H__
#define __FITOS_QUEUE_H__

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#define SYNCS_ERROR -2
#define MAIN_ERROR -1
#define ENDLESS 1
#define EMPTY 0

#define FALSE 0
#define TRUE 1

typedef struct _Link {
	int len;
	int count;
} Link;

typedef struct _Node {
	char value[100];
	struct _Node* next;
	pthread_mutex_t sync;
} Node;

typedef struct _Storage {
	pthread_mutex_t sync;

	Node* first;
	Node* last;

	int count;
	int max_count;
} Storage;

Storage* storage_init(int max_count);
void storage_destroy(Storage* q);
int storage_add(Storage* q, char* val);
int storage_get(Storage* q, char** val);

#endif		// __FITOS_QUEUE_H__
