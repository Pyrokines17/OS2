#define _GNU_SOURCE

#include <pthread.h>
#include <assert.h>

#include "queue.h"

void* qmonitor(void *arg) {
	queue_t *q = (queue_t *)arg;

	printf("qmonitor: [%d %d %d]\n", getpid(), getppid(), gettid());

	while (ENDLESS) {
		queue_print_stats(q);
		sleep(1);
	}

	return NULL;
}

queue_t* queue_init(int max_count) {
	int err;

	queue_t *q = malloc(sizeof(queue_t));

	if (q == NULL) {
		fprintf(stderr, "Cannot allocate memory for a queue\n");
		return NULL;
	}

    spinlock_init(&q->spinlock);

	q->first = NULL;
	q->last = NULL;
	q->max_count = max_count;
	q->count = 0;

	q->add_attempts = q->get_attempts = 0;
	q->add_count = q->get_count = 0;

	err = pthread_create(&q->qmonitor_tid, NULL, qmonitor, q);

	if (err != EXIT_SUCCESS) {
		fprintf(stderr, "queue_init: pthread_create() failed: %s\n", strerror(err));
		free(q);
		return NULL;
	}

	return q;
}

int queue_destroy(queue_t *q) {
    spinlock_lock(&q->spinlock);

	qnode_t *tmp;
	int len, stat;

	stat = pthread_cancel(q->qmonitor_tid);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_destroy: pthread_cancel() failed: %s\n", strerror(stat));
		spinlock_unlock(&q->spinlock);
		return stat;
	}

	stat = pthread_join(q->qmonitor_tid, NULL);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_destroy: pthread_join() failed: %s\n", strerror(stat));
        spinlock_unlock(&q->spinlock);
		return stat;
	}

	len = q->count;

	for (int i = 0; i < len; ++i) {
		tmp = q->first;
		q->first = q->first->next;
		free(tmp);
	}

	spinlock_unlock(&q->spinlock);
	free(q);

	return EXIT_SUCCESS;
}

int queue_add(queue_t *q, int val) {
	spinlock_lock(&q->spinlock);

	q->add_attempts++;

	assert(q->count <= q->max_count);

	if (q->count == q->max_count) {
		spinlock_unlock(&q->spinlock);
		return FALSE;
	}

	qnode_t *new = malloc(sizeof(qnode_t));
	
	if (new == NULL) {
		fprintf(stderr, "Cannot allocate memory for new node\n");
		spinlock_unlock(&q->spinlock);
		return MALLOC_ERROR;
	}

	new->val = val;
	new->next = NULL;

	if (q->first == NULL) {
		q->first = q->last = new;
	} else {
		q->last->next = new;
		q->last = q->last->next;
	}

	q->count++;
	q->add_count++;

	spinlock_unlock(&q->spinlock);

	return TRUE;
}

int queue_get(queue_t *q, int *val) {
    spinlock_lock(&q->spinlock);

	q->get_attempts++;

	assert(q->count >= 0);

	if (q->count == 0) {
		spinlock_unlock(&q->spinlock);
		return FALSE;
	}

	qnode_t *tmp = q->first;

	*val = tmp->val;
	q->first = q->first->next;

	free(tmp);

	q->count--;
	q->get_count++;

    spinlock_unlock(&q->spinlock);

	return TRUE;
}

void queue_print_stats(queue_t *q) {
    spinlock_lock(&q->spinlock);

	printf("queue stats: current size %d; attempts: (%ld %ld %ld); counts (%ld %ld %ld)\n",
		q->count,
		q->add_attempts, q->get_attempts, q->add_attempts - q->get_attempts,
		q->add_count, q->get_count, q->add_count -q->get_count);

	spinlock_unlock(&q->spinlock);
}
