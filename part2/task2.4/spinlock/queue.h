#ifndef __FITOS_QUEUE_H__
#define __FITOS_QUEUE_H__

#define _GNU_SOURCE

#include "spinlock.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define UNLOCK_ERROR -3
#define MALLOC_ERROR -2
#define MAIN_ERROR -1

#define ENDLESS 1
#define EMPTY 0

#define FALSE 0
#define TRUE 1

typedef struct _QueueNode {
	int val;
	struct _QueueNode *next;
} qnode_t;

typedef struct _Queue {
	qnode_t *first;
	qnode_t *last;

	pthread_t qmonitor_tid;

	int count;
	int max_count;

	// queue statistics
	
	long add_attempts;
	long get_attempts;
	long add_count;
	long get_count;

	spinlock_t spinlock;
} queue_t;

typedef struct _SyncQueue {
	queue_t* queue;
} syncQueue_t;

queue_t* queue_init(int max_count);
int queue_destroy(queue_t *q);
int queue_add(queue_t *q, int val);
int queue_get(queue_t *q, int *val);
void queue_print_stats(queue_t *q);

#endif		// __FITOS_QUEUE_H__