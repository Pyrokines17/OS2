#define _GNU_SOURCE

#include "depend.h"
#include "storage-rw.h"

size_t increase_v = 0;
size_t increase_c;

size_t decrease_v = 0;
size_t decrease_c;

size_t equal_v = 0;
size_t equal_c;

size_t swap_v = 0;
size_t counter = 0;

pthread_rwlock_t increase_sync;
pthread_rwlock_t decrease_sync;
pthread_rwlock_t equal_sync;
pthread_rwlock_t swap_sync;

int my_pthread_rwlock_wrlock(pthread_rwlock_t* rwlock) {
	int stat = pthread_rwlock_wrlock(rwlock);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "my_pthread_rwlock_wrlock: pthread_rwlock_wrlock() failed: %s\n", strerror(stat));
		return stat;
	}

	return EXIT_SUCCESS;
}

int my_pthread_rwlock_rdlock(pthread_rwlock_t* rwlock) {
	int stat = pthread_rwlock_rdlock(rwlock);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "my_pthread_rwlock_rdlock: pthread_rwlock_rdlock() failed: %s\n", strerror(stat));
		return stat;
	}

	return EXIT_SUCCESS;
}

int my_pthread_rwlock_unlock(pthread_rwlock_t* rwlock) {
	int stat = pthread_rwlock_unlock(rwlock);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "my_pthread_rwlock_unlock: pthread_rwlock_unlock() failed: %s\n", strerror(stat));
		return stat;
	}

	return EXIT_SUCCESS;
}

int my_pthread_rwlock_destroy(pthread_rwlock_t* rwlock) {
	int stat = pthread_rwlock_destroy(rwlock);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "my_pthread_rwlock_destroy: pthread_rwlock_destroy() failed: %s\n", strerror(stat));
		return stat;
	}

	return EXIT_SUCCESS;
}

int show_storage(Storage* s) {
	++counter;

	printf("--------------------\n");
	printf("Counter: %ld\n", counter);

	int stat = my_pthread_rwlock_rdlock(&swap_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_rwlock_rdlock() id 1 failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	printf("Swap iter: %ld\n\n", swap_v);
	
	stat = my_pthread_rwlock_unlock(&swap_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_rwlock_unlock() id 1 failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	stat = my_pthread_rwlock_rdlock(&increase_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_rwlock_rdlock() id 2 failed: %s\n", strerror(stat));
		return  EXIT_FAILURE;
	}

	printf("Increase iter: %ld\n", increase_v);
	printf("Increase count: %ld\n\n", increase_c);
	
	stat = my_pthread_rwlock_unlock(&increase_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_rwlock_unlock() id 2 failed: %s\n", strerror(stat));
		return  EXIT_FAILURE;
	}

	stat = my_pthread_rwlock_rdlock(&decrease_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_rwlock_rdlock() id 3 failed: %s\n", strerror(stat));
		return  EXIT_FAILURE;
	}

	printf("Decrease iter: %ld\n", decrease_v);
	printf("Decrease count: %ld\n\n", decrease_c);
	
	stat = my_pthread_rwlock_unlock(&decrease_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_rwlock_unlock() id 3 failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	stat = my_pthread_rwlock_rdlock(&equal_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_rwlock_rdlock() id 4 failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	printf("Equal iter: %ld\n", equal_v);
	printf("Equal count: %ld\n", equal_c);
	
	stat = my_pthread_rwlock_unlock(&equal_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage: my_pthread_rwlock_unlock() id 4 failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	printf("--------------------\n");

	return EXIT_SUCCESS;
}

int change_increase(int count) {
	int stat = my_pthread_rwlock_wrlock(&increase_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_increase: my_pthread_rwlock_wrlock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}
	
	++increase_v;
	increase_c = count;
	
	stat = my_pthread_rwlock_unlock(&increase_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_increase: my_pthread_rwlock_unlock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int change_decrease(int count) {
	int stat = my_pthread_rwlock_wrlock(&decrease_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_decrease: my_pthread_rwlock_wrlock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}
	
	++decrease_v;
	decrease_c = count;
	
	stat = my_pthread_rwlock_unlock(&decrease_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_decrease: my_pthread_rwlock_unlock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int change_equal(int count) {
	int stat = my_pthread_rwlock_wrlock(&equal_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_equal: my_pthread_rwlock_wrlock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}
	
	++equal_v;
	equal_c = count;
	
	stat = my_pthread_rwlock_unlock(&equal_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_equal: my_pthread_rwlock_unlock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int change_swap() {
	int stat = my_pthread_rwlock_wrlock(&swap_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_swap: my_pthread_rwlock_wrlock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}
	
	++swap_v;
	
	stat = my_pthread_rwlock_unlock(&swap_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "change_swap: my_pthread_rwlock_unlock() failed: %s\n", strerror(stat));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int init_syncs() {
	int stat, statOt;

	stat = pthread_rwlock_init(&swap_sync, NULL);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "init_syncs: pthread_rwlock_init() id 1 failed: %s\n", strerror(stat));
		return SYNCS_ERROR;
	}

	stat = pthread_rwlock_init(&increase_sync, NULL);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "init_syncs: pthread_rwlock_init() id 2 failed: %s\n", strerror(stat));

		statOt = my_pthread_rwlock_destroy(&swap_sync);

		if (statOt != EXIT_SUCCESS) {
			fprintf(stderr, "init_syncs: my_pthread_rwlock_destroy() id 1 failed: %s\n", strerror(statOt));
		}

		return SYNCS_ERROR;
	}

	stat = pthread_rwlock_init(&decrease_sync, NULL);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "init_syncs: pthread_rwlock_init() id 3 failed: %s\n", strerror(stat));

		statOt = my_pthread_rwlock_destroy(&increase_sync);

		if (statOt != EXIT_SUCCESS) {
			fprintf(stderr, "init_syncs: my_pthread_rwlock_destroy() id 2 failed: %s\n", strerror(statOt));
		}

		statOt = my_pthread_rwlock_destroy(&swap_sync);

		if (statOt != EXIT_SUCCESS) {
			fprintf(stderr, "init_syncs: my_pthread_rwlock_destroy() id 3 failed: %s\n", strerror(statOt));
		}

		return SYNCS_ERROR;
	}

	stat = pthread_rwlock_init(&equal_sync, NULL);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "init_syncs: pthread_rwlock_init() id 4 failed: %s\n", strerror(stat));

		statOt = my_pthread_rwlock_destroy(&decrease_sync);

		if (statOt != EXIT_SUCCESS) {
			fprintf(stderr, "init_syncs: my_pthread_rwlock_destroy() id 4 failed: %s\n", strerror(statOt));
		}

		statOt = my_pthread_rwlock_destroy(&increase_sync);

		if (statOt != EXIT_SUCCESS) {
			fprintf(stderr, "init_syncs: my_pthread_rwlock_destroy() id 5 failed: %s\n", strerror(statOt));
		}

		statOt = my_pthread_rwlock_destroy(&swap_sync);

		if (statOt != EXIT_SUCCESS) {
			fprintf(stderr, "init_syncs: my_pthread_rwlock_destroy() id 6 failed: %s\n", strerror(statOt));
		}

		return SYNCS_ERROR;
	}

	return EXIT_SUCCESS;
}

void destroy_syncs() {
	int stat = my_pthread_rwlock_destroy(&swap_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "destroy_syncs: my_pthread_rwlock_destroy() id 1 failed: %s\n", strerror(stat));
	}

	stat = my_pthread_rwlock_destroy(&increase_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "destroy_syncs: my_pthread_rwlock_destroy() id 2 failed: %s\n", strerror(stat));
	}

	stat = my_pthread_rwlock_destroy(&decrease_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "destroy_syncs: my_pthread_rwlock_destroy() id 3 failed: %s\n", strerror(stat));
	}

	stat = my_pthread_rwlock_destroy(&equal_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "destroy_syncs: my_pthread_rwlock_destroy() id 4 failed: %s\n", strerror(stat));
	}
}
