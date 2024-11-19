#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>

#include "storage-mut.h"

Storage* storage_init(int max_count) {
	Storage* s = malloc(sizeof(Storage));

	if (s == NULL) {
		fprintf(stderr, "Cannot allocate memory for a queue\n");
		return NULL;
	}

	s->first = NULL;
	s->last = NULL;

	s->count = 0;
	s->max_count = max_count;

	int stat = pthread_mutex_init(&s->sync, NULL);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_init: pthread_mutex_init() failed: %s\n", strerror(stat));
		return NULL;
	}

	return s;
}

void storage_destroy(Storage* s) {
	int stat, len;
	Node* tmp;

	len = s->count;

	for (int i = 0; i < len; ++i) {
		tmp = s->first;
		s->first = s->first->next;

		stat = pthread_mutex_destroy(&tmp->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "queue_destroy: pthread_mutex_destroy() failed: %s\n", strerror(stat));
			return;
		}

		free(tmp);
	}

	stat = pthread_mutex_destroy(&s->sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_destroy: pthread_mutex_destroy() failed: %s\n", strerror(stat));
		return;
	}

	free(s);

	return;
}

int storage_add(Storage* s, char* val) {
	assert(s->count <= s->max_count);

	if (s->count == s->max_count) {
		return FALSE;
	}

	Node* new = malloc(sizeof(Node));

	if (new == NULL) {
		fprintf(stderr, "Cannot allocate memory for new node\n");
		return FALSE;
	}

	strcpy(new->value, val);
	new->next = NULL;

	int stat = pthread_mutex_init(&new->sync, NULL);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_add: pthread_mutex_init() failed: %s\n", strerror(stat));
		return FALSE;
	}

	if (s->first == NULL) {
		s->first = s->last = new;
	} else {
		s->last->next = new;
		s->last = s->last->next;
	}

	s->count++;

	return TRUE;
}

int storage_get(Storage* s, char** val) {
	assert(s->count >= 0);

	if (s->count == 0) {
		return FALSE;
	}

	Node* tmp = s->first;
	s->first = s->first->next;

	strcpy(*val, tmp->value);

	int stat = pthread_mutex_destroy(&tmp->sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "queue_get: pthread_mutex_destroy() failed: %s\n", strerror(stat));
		return FALSE;
	}

	free(tmp);

	s->count--;

	return TRUE;
}
