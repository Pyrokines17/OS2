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
#include "storage-mut.h"

#define STOR_SIZE 100

int main() {
	pthread_t inc_tid, dec_tid, eq_tid;
	pthread_t sw_tids[3];
	int stat;

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
		stat = storage_destroy(s);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "storage_destroy() id 1 failed: %d\n", stat);
		}

		return MAIN_ERROR;
	}

	stat = pthread_create(&inc_tid, NULL, increase, s);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_create() id 1 failed: %s\n", strerror(stat));
		destroy_syncs();
		stat = storage_destroy(s);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "storage_destroy() id 2 failed: %d\n", stat);
		}

		return MAIN_ERROR;
	}

	stat = pthread_create(&dec_tid, NULL, decrease, s);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_create() id 2 failed: %s\n", strerror(stat));
		destroy_syncs();
		stat = storage_destroy(s);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "storage_destroy() id 3 failed: %d\n", stat);
		}

		return MAIN_ERROR;
	}

	stat = pthread_create(&eq_tid, NULL, equal, s);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_create() id 3 failed: %s\n", strerror(stat));
		destroy_syncs();
		stat = storage_destroy(s);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "storage_destroy() id 4 failed: %d\n", stat);
		}

		return MAIN_ERROR;
	}

	for (int i = 0; i < 3; ++i) {
		stat = pthread_create(&sw_tids[i], NULL, swap, s);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "pthread_create() id 4 failed: %s\n", strerror(stat));
			destroy_syncs();
			stat = storage_destroy(s);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "storage_destroy() id 5 failed: %d\n", stat);
			}

			return MAIN_ERROR;
		}
	}

	while (TRUE) {
		stat = show_storage(s);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "show_storage() failed: %d\n", stat);
			destroy_syncs();
			stat = storage_destroy(s);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "storage_destroy() id 6 failed: %d\n", stat);
			}

			return MAIN_ERROR;
		}
		
		sleep(1);
	}

	return EXIT_SUCCESS;
}
