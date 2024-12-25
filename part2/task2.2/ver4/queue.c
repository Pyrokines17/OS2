#define _GNU_SOURCE

#include <pthread.h>
#include <assert.h>

#include "queue.h"

int my_sem_post(queue_t* q) {
	int statOt = sem_post(&q->sem);

	if (statOt != EXIT_SUCCESS) {
		fprintf(stderr, "sem_post() failed: %s\n", strerror(statOt));
		return UNLOCK_ERROR;
	}

	return EXIT_SUCCESS;
}

int my_sem_destroy(queue_t* q) {
	int err = sem_destroy(&q->sem);
		
	if (err != EXIT_SUCCESS) {
		fprintf(stderr, "sem_destroy() failed: %s\n", strerror(err));
		return SEM_ERROR;
	}

	return EXIT_SUCCESS;
}

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

	err = sem_init(&q->sem, 0, 1);

	if (err != EXIT_SUCCESS) {
		fprintf(stderr, "queue_init: sem_init() failed: %s\n", strerror(err));
		free(q);
		return NULL;
	}

	q->first = NULL;
	q->last = NULL;
	q->max_count = max_count;
	q->count = 0;

	q->add_attempts = q->get_attempts = 0;
	q->add_count = q->get_count = 0;

	err = pthread_create(&q->qmonitor_tid, NULL, qmonitor, q);

	if (err != EXIT_SUCCESS) {
		fprintf(stderr, "queue_init: pthread_create() failed: %s\n", strerror(err));
		my_sem_destroy(q);
		free(q);
		return NULL;
	}

	return q;
}

int queue_destroy(queue_t *q) {
	int stat = sem_wait(&q->sem);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_destroy: sem_wait() failed: %s\n", strerror(stat));
		return stat;
	}

	qnode_t *tmp;
	int len;

	stat = pthread_cancel(q->qmonitor_tid);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_destroy: pthread_cancel() failed: %s\n", strerror(stat));
		my_sem_post(q);
		return stat;
	}

	stat = pthread_join(q->qmonitor_tid, NULL);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_destroy: pthread_join() failed: %s\n", strerror(stat));
		my_sem_post(q);
		return stat;
	}

	len = q->count;

	for (int i = 0; i < len; ++i) {
		tmp = q->first;
		q->first = q->first->next;
		free(tmp);
	}

	stat = sem_post(&q->sem);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_destroy: sem_post() id 3 failed: %s\n", strerror(stat));
		return stat;
	}

	stat = sem_destroy(&q->sem);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_destroy: sem_destroy() failed: %s\n", strerror(stat));
		return stat;
	}

	free(q);

	return EXIT_SUCCESS;
}

int queue_add(queue_t *q, int val) {
	int stat = sem_wait(&q->sem);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_add: sem_wait() failed: %s\n", strerror(stat));
		return LOCK_ERROR;
	}

	q->add_attempts++;

	assert(q->count <= q->max_count);

	if (q->count == q->max_count) {
		stat = my_sem_post(q);

		if (stat != EXIT_SUCCESS) {
			return stat;
		}

		return FALSE;
	}

	qnode_t *new = malloc(sizeof(qnode_t));
	
	if (new == NULL) {
		fprintf(stderr, "Cannot allocate memory for new node\n");
		my_sem_post(q);
		return EXIT_FAILURE;
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

	stat = sem_post(&q->sem);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_add: sem_post() id 3 failed: %s\n", strerror(stat));
		return UNLOCK_ERROR;
	}

	return TRUE;
}

int queue_get(queue_t *q, int *val) {
	int stat = sem_wait(&q->sem);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_get: sem_wait() failed: %s\n", strerror(stat));
		return LOCK_ERROR;
	}

	q->get_attempts++;

	assert(q->count >= 0);

	if (q->count == 0) {
		stat = my_sem_post(q);

		if (stat != EXIT_SUCCESS) {
			return stat;
		}

		return FALSE;
	}

	qnode_t *tmp = q->first;

	*val = tmp->val;
	q->first = q->first->next;

	free(tmp);

	q->count--;
	q->get_count++;

	stat = sem_post(&q->sem);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_get: sem_post() id 2 failed: %s\n", strerror(stat));
		return UNLOCK_ERROR;
	}

	return TRUE;
}

void queue_print_stats(queue_t *q) {
	int stat = sem_wait(&q->sem);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_print_stats: sem_wait() failed: %s\n", strerror(stat));
		pthread_exit(NULL);
	}

	printf("queue stats: current size %d; attempts: (%ld %ld %ld); counts (%ld %ld %ld)\n",
		q->count,
		q->add_attempts, q->get_attempts, q->add_attempts - q->get_attempts,
		q->add_count, q->get_count, q->add_count -q->get_count);

	stat = sem_post(&q->sem);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_print_stats: sem_post() failed: %s\n", strerror(stat));
		pthread_exit(NULL);
	}
}

