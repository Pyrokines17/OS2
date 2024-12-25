#define _GNU_SOURCE

#include "depend.h"
#include "storage-mut.h"

size_t increase_v = 0;
size_t increase_c;

size_t decrease_v = 0;
size_t decrease_c;

size_t equal_v = 0;
size_t equal_c;

size_t swap_v = 0;
size_t counter = 0;

pthread_mutex_t increase_sync;
pthread_mutex_t decrease_sync;
pthread_mutex_t equal_sync;
pthread_mutex_t swap_sync;

int my_pthread_mutex_lock(pthread_mutex_t* mutex) {
	int stat = pthread_mutex_lock(mutex);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "my_pthread_mutex_lock: pthread_mutex_lock() failed: %s\n", strerror(stat));
		return stat;
	}

	return EXIT_SUCCESS;
}

int my_pthread_mutex_unlock(pthread_mutex_t* mutex) {
	int stat = pthread_mutex_unlock(mutex);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "my_pthread_mutex_unlock: pthread_mutex_unlock() failed: %s\n", strerror(stat));
		return stat;
	}

	return EXIT_SUCCESS;
}

int my_pthread_mutex_destroy(pthread_mutex_t* mutex) {
	int stat = pthread_mutex_destroy(mutex);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "my_pthread_mutex_destroy: pthread_mutex_destroy() failed: %s\n", strerror(stat));
		return stat;
	}

	return EXIT_SUCCESS;
}

int show_storage(Storage* s) {
	++counter;

	printf("--------------------\n");
	printf("Counter: %ld\n", counter);

	int stat = my_pthread_mutex_lock(&swap_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_mutex_lock() id 1 failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	printf("Swap iter: %ld\n\n", swap_v);
	
	stat = my_pthread_mutex_unlock(&swap_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_mutex_unlock() id 1 failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	stat = my_pthread_mutex_lock(&increase_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_mutex_lock() id 2 failed: %s\n", strerror(stat));
		return  EXIT_FAILURE;
	}

	printf("Increase iter: %ld\n", increase_v);
	printf("Increase count: %ld\n\n", increase_c);
	
	stat = my_pthread_mutex_unlock(&increase_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_mutex_unlock() id 2 failed: %s\n", strerror(stat));
		return  EXIT_FAILURE;
	}

	stat = my_pthread_mutex_lock(&decrease_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_mutex_lock() id 3 failed: %s\n", strerror(stat));
		return  EXIT_FAILURE;
	}

	printf("Decrease iter: %ld\n", decrease_v);
	printf("Decrease count: %ld\n\n", decrease_c);
	
	stat = my_pthread_mutex_unlock(&decrease_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_mutex_unlock() id 3 failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	stat = my_pthread_mutex_lock(&equal_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_mutex_lock() id 4 failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	printf("Equal iter: %ld\n", equal_v);
	printf("Equal count: %ld\n", equal_c);
	
	stat = my_pthread_mutex_unlock(&equal_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_mutex_unlock() id 4 failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	printf("--------------------\n");

	return EXIT_SUCCESS;
}

int change_increase(int count) {
	int stat = my_pthread_mutex_lock(&increase_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_increase: my_pthread_mutex_lock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}
	
	++increase_v;
	increase_c = count;
	
	stat = my_pthread_mutex_unlock(&increase_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_increase: my_pthread_mutex_unlock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int change_decrease(int count) {
	int stat = my_pthread_mutex_lock(&decrease_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_decrease: my_pthread_mutex_lock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}
	
	++decrease_v;
	decrease_c = count;
	
	stat = my_pthread_mutex_unlock(&decrease_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_decrease: my_pthread_mutex_unlock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int change_equal(int count) {
	int stat = my_pthread_mutex_lock(&equal_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_equal: my_pthread_mutex_lock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}
	
	++equal_v;
	equal_c = count;
	
	stat = my_pthread_mutex_unlock(&equal_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_equal: my_pthread_mutex_unlock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int change_swap() {
	int stat = my_pthread_mutex_lock(&swap_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_swap: my_pthread_mutex_lock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}
	
	++swap_v;
	
	stat = my_pthread_mutex_unlock(&swap_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_swap: my_pthread_mutex_unlock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int init_syncs() {
	int stat;

	stat = pthread_mutex_init(&swap_sync, NULL);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "init_syncs: pthread_mutex_init() id 1 failed: %s\n", strerror(stat));
		return SYNCS_ERROR;
	}

	stat = pthread_mutex_init(&increase_sync, NULL);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "init_syncs: pthread_mutex_init() id 2 failed: %s\n", strerror(stat));
		my_pthread_mutex_destroy(&swap_sync);
		return SYNCS_ERROR;
	}

	stat = pthread_mutex_init(&decrease_sync, NULL);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "init_syncs: pthread_mutex_init() id 3 failed: %s\n", strerror(stat));
		my_pthread_mutex_destroy(&increase_sync);
		my_pthread_mutex_destroy(&swap_sync);
		return SYNCS_ERROR;
	}

	stat = pthread_mutex_init(&equal_sync, NULL);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "init_syncs: pthread_mutex_init() id 4 failed: %s\n", strerror(stat));
		my_pthread_mutex_destroy(&decrease_sync);
		my_pthread_mutex_destroy(&increase_sync);
		my_pthread_mutex_destroy(&swap_sync);
		return SYNCS_ERROR;
	}

	return EXIT_SUCCESS;
}

void destroy_syncs() {
	int stat = pthread_mutex_destroy(&swap_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "destroy_syncs: pthread_mutex_destroy() id 1 failed: %s\n", strerror(stat));
	}

	stat = pthread_mutex_destroy(&increase_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "destroy_syncs: pthread_mutex_destroy() id 2 failed: %s\n", strerror(stat));
	}

	stat = pthread_mutex_destroy(&decrease_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "destroy_syncs: pthread_mutex_destroy() id 3 failed: %s\n", strerror(stat));
	}

	stat = pthread_mutex_destroy(&equal_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "destroy_syncs: pthread_mutex_destroy() id 4 failed: %s\n", strerror(stat));
	}
}
