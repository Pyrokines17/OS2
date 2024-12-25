#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>

#include "storage-spin.h"

void my_pthread_spin_unlock_st(pthread_spinlock_t* lock) {
	int stat = pthread_spin_unlock(lock);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "my_pthread_spin_unlock_st: pthread_spin_unlock() failed: %s\n", strerror(stat));
	}
}

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

	int stat = pthread_spin_init(&s->sync, PTHREAD_PROCESS_PRIVATE);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "storage_init: pthread_spin_init() failed: %s\n", strerror(stat));
		free(s);
		return NULL;
	}

	return s;
}

int storage_destroy(Storage* s) {
	int stat, len;
	Node* tmp;

	stat = pthread_spin_lock(&s->sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "storage_destroy: pthread_spin_lock() failed: %s\n", strerror(stat));
		return LOCK_ERROR;
	}

	len = s->count;

	for (int i = 0; i < len; ++i) {
		tmp = s->first;
		s->first = s->first->next;

		stat = pthread_spin_destroy(&tmp->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "storage_destroy: pthread_spin_destroy() id 1 failed: %s\n", strerror(stat));
			my_pthread_spin_unlock_st(&s->sync);
			return EXIT_FAILURE;
		}

		free(tmp);
	}

	stat = pthread_spin_unlock(&s->sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "storage_destroy: pthread_spin_unlock() failed: %s\n", strerror(stat));
		return LOCK_ERROR;
	}

	stat = pthread_spin_destroy(&s->sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "storage_destroy: pthread_spin_destroy() id 2 failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	free(s);

	return EXIT_SUCCESS;
}

int storage_add(Storage* s, char* val) {
	int stat = pthread_spin_lock(&s->sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "storage_add: pthread_spin_lock() failed: %s\n", strerror(stat));
		return LOCK_ERROR;
	}

	assert(s->count <= s->max_count);

	if (s->count == s->max_count) {
		my_pthread_spin_unlock_st(&s->sync);
		return FALSE;
	}

	Node* new = malloc(sizeof(Node));

	if (new == NULL) {
		fprintf(stderr, "Cannot allocate memory for new node\n");
		my_pthread_spin_unlock_st(&s->sync);
		return MALLOC_ERROR;
	}

	new->len = strlen(val);
	strcpy(new->value, val);
	new->next = NULL;

	stat = pthread_spin_init(&new->sync, PTHREAD_PROCESS_PRIVATE);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "storage_add: pthread_spin_init() failed: %s\n", strerror(stat));
		free(new);
		my_pthread_spin_unlock_st(&s->sync);
		return INIT_ERROR;
	}

	if (s->first == NULL) {
		s->first = s->last = new;
	} else {
		s->last->next = new;
		s->last = s->last->next;
	}

	s->count++;

	stat = pthread_spin_unlock(&s->sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "storage_add: pthread_spin_unlock() failed: %s\n", strerror(stat));
		return LOCK_ERROR;
	}

	return TRUE;
}

int storage_get(Storage* s, char** val) {
	int stat = pthread_spin_lock(&s->sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "storage_get: pthread_spin_lock() failed: %s\n", strerror(stat));
		return LOCK_ERROR;
	}

	assert(s->count >= 0);

	if (s->count == 0) {
		my_pthread_spin_unlock_st(&s->sync);
		return FALSE;
	}

	Node* tmp = s->first;
	s->first = s->first->next;

	strcpy(*val, tmp->value);

	stat = pthread_spin_destroy(&tmp->sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "storage_get: pthread_spin_destroy() failed: %s\n", strerror(stat));
		my_pthread_spin_unlock_st(&s->sync);
		return DESTROY_ERROR;
	}

	free(tmp);

	s->count--;

	stat = pthread_spin_unlock(&s->sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "storage_get: pthread_spin_unlock() failed: %s\n", strerror(stat));
		return LOCK_ERROR;
	}

	return TRUE;
}

int fill_storage(Storage* s) {
	int len = s->max_count, stat;
	int part = len / PCOUNT;
	char str[100] = "test";

	for (int i = 0; i < PCOUNT; ++i) {
		for (int j = 0; j < part; ++j) {
			stat = storage_add(s, str);

			if (stat != TRUE && stat != FALSE) {
				fprintf(stderr, "fill_storage: storage_add() failed: %d\n", stat);
				return stat;
			}
		}

		strcat(str, "test");
	}
	
	return EXIT_SUCCESS;
}
