#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <sched.h>
#include <pthread.h>

#include "swap.h"
#include "depend.h"
#include "compare.h"
#include "storage-spin.h"

#define STOR_SIZE 1000
#define BORDER 60

int my_storage_destroy(Storage* s) {
	int stat = storage_destroy(s);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "storage_destroy() failed: %d\n", stat);
		return stat;
	}

	return EXIT_SUCCESS;
}

int cancel_join_threads(pthread_t* tids, int count) {
	int stat;

	for (int i = 0; i < count; ++i) {
		stat = pthread_cancel(tids[i]);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "pthread_cancel() id %d failed: %s\n", i, strerror(stat));
			return EXIT_FAILURE;
		}

		stat = pthread_join(tids[i], NULL);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "pthread_join() id %d failed: %s\n", i, strerror(stat));
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

int main() {
	pthread_t sw_tids[3], comp_tids[3];
	int stat, counter = 0;

	stat = init_syncs();

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "init_syncs() failed: %d\n", stat);
		return MAIN_ERROR;
	}

	Storage* s = storage_init(STOR_SIZE);

	if (s == NULL) {
		fprintf(stderr, "Cannot allocate memory for a storage\n");
		destroy_syncs();
		return MAIN_ERROR;
	}

	stat = fill_storage(s);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "fill_storage() failed: %d\n", stat);
		destroy_syncs();
		my_storage_destroy(s);
		return MAIN_ERROR;
	}

	stat = pthread_create(&comp_tids[0], NULL, increase, s);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_create() id 1 failed: %s\n", strerror(stat));
		destroy_syncs();
		my_storage_destroy(s);
		return MAIN_ERROR;
	}

	stat = pthread_create(&comp_tids[1], NULL, decrease, s);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_create() id 2 failed: %s\n", strerror(stat));
		destroy_syncs();
		my_storage_destroy(s);
		return MAIN_ERROR;
	}

	stat = pthread_create(&comp_tids[2], NULL, equal, s);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_create() id 3 failed: %s\n", strerror(stat));
		destroy_syncs();
		my_storage_destroy(s);
		return MAIN_ERROR;
	}

	for (int i = 0; i < 3; ++i) {
		stat = pthread_create(&sw_tids[i], NULL, swap, s);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "pthread_create() id 4 failed: %s\n", strerror(stat));
			destroy_syncs();
			my_storage_destroy(s);
			return MAIN_ERROR;
		}
	}

	while (counter < BORDER) {
		stat = show_storage(s);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "show_storage() failed: %d\n", stat);
			destroy_syncs();
			my_storage_destroy(s);
			return MAIN_ERROR;
		}
		
		sleep(1);

		++counter;
	}

	stat = cancel_join_threads(comp_tids, 3);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "cancel_join_threads() id 1 failed: %d\n", stat);
		destroy_syncs();
		my_storage_destroy(s);
		return MAIN_ERROR;
	}

	stat = cancel_join_threads(sw_tids, 3);
	
	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "cancel_join_threads() id 2 failed: %d\n", stat);
		destroy_syncs();
		my_storage_destroy(s);
		return MAIN_ERROR;
	}

	stat = storage_destroy(s);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "storage_destroy() id 9 failed: %d\n", stat);
		destroy_syncs();
		return MAIN_ERROR;
	}

	destroy_syncs();

	return EXIT_SUCCESS;
}
