#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <sched.h>
#include <pthread.h>

#include "storage-spe.h"

#define RED "\033[41m"
#define NOCOLOR "\033[0m"

size_t increase_v = 0;
size_t increase_c;

size_t decrease_v = 0;
size_t decrease_c;

size_t equal_v = 0;
size_t equal_c;

size_t swap_v = 0;

pthread_spinlock_t increase_sync;
pthread_spinlock_t decrease_sync;
pthread_spinlock_t equal_sync;
pthread_spinlock_t swap_sync;

int my_pthread_spin_lock(pthread_spinlock_t* spinlock) {
	int stat = pthread_spin_lock(spinlock);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_spin_lock() failed: %s\n", strerror(stat));
		return stat;
	}

	return EXIT_SUCCESS;
}

int my_pthread_spin_unlock(pthread_spinlock_t* spinlock) {
	int stat = pthread_spin_unlock(spinlock);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_spin_unlock() failed: %s\n", strerror(stat));
		return stat;
	}

	return EXIT_SUCCESS;
}

void show_storage(Storage* s) {
	int stat = my_pthread_spin_lock(&swap_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage() failed: %s\n", strerror(stat));
		return;
	}

	printf("Swap iter: %ld\n", swap_v);
	
	stat = my_pthread_spin_unlock(&swap_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage() failed: %s\n", strerror(stat));
		return;
	}

	stat = my_pthread_spin_lock(&increase_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage() failed: %s\n", strerror(stat));
		return;
	}

	printf("Increase iter: %ld\n", increase_v);
	printf("Increase count: %ld\n", increase_c);
	
	stat = my_pthread_spin_unlock(&increase_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage() failed: %s\n", strerror(stat));
		return;
	}

	stat = my_pthread_spin_lock(&decrease_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage() failed: %s\n", strerror(stat));
		return;
	}

	printf("Decrease iter: %ld\n", decrease_v);
	printf("Decrease count: %ld\n", decrease_c);
	
	stat = my_pthread_spin_unlock(&decrease_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage() failed: %s\n", strerror(stat));
		return;
	}

	stat = my_pthread_spin_lock(&equal_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage() failed: %s\n", strerror(stat));
		return;
	}

	printf("Equal iter: %ld\n", equal_v);
	printf("Equal count: %ld\n", equal_c);
	
	stat = my_pthread_spin_unlock(&equal_sync);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "show_storage() failed: %s\n", strerror(stat));
		return;
	}
}

void* increase(void* arg) {
	Storage* s = (Storage*)arg;
	int count, len, clinks;
	int flag = 0, stat;
	Node *tmp, *buf;
	Link *links;

	links = malloc(10 * sizeof(Link));

	if (links == NULL) {
		fprintf(stderr, "increase() failed: %s\n", strerror(errno));
		return NULL;
	}

	while (TRUE) {
		stat = my_pthread_spin_lock(&s->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "increase() failed: %s\n", strerror(stat));
			return NULL;
		}

		tmp = s->first;
		
		stat = my_pthread_spin_unlock(&s->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "increase() failed: %s\n", strerror(stat));
			return NULL;
		}

		clinks = count = 0;
		len = strlen(tmp->value);

		links[clinks].len = len;
		links[clinks].count = 1;
		++clinks;

		stat = my_pthread_spin_lock(&tmp->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "increase() failed: %s\n", strerror(stat));
			return NULL;
		}

		buf = tmp->next;
		
		stat = my_pthread_spin_unlock(&tmp->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "increase() failed: %s\n", strerror(stat));
			return NULL;
		}

		tmp = buf;

		while (tmp != NULL) {
			len = strlen(tmp->value);

			for (int i = 0; i < clinks; ++i) {
				if (links[i].len < len) {
					count += links[i].count;
				}

				if (links[i].len == len) {
					++links[i].count;
					flag = 1;
				}
			}

			if (flag == 0) {
				links[clinks].len = len;
				links[clinks].count = 1;
				++clinks;
			}

			flag = 0;

			stat = my_pthread_spin_lock(&tmp->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "increase() failed: %s\n", strerror(stat));
				return NULL;
			}

			buf = tmp->next;
			stat = my_pthread_spin_unlock(&tmp->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "increase() failed: %s\n", strerror(stat));
				return NULL;
			}

			tmp = buf;
		}

		stat = my_pthread_spin_lock(&increase_sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "increase() failed: %s\n", strerror(stat));
			return NULL;
		}

		++increase_v;
		increase_c = count;
		
		stat = my_pthread_spin_unlock(&increase_sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "increase() failed: %s\n", strerror(stat));
			return NULL;
		}
	}

	free(links);
}

void* decrease(void* arg) {
	Storage* s = (Storage*)arg;
	int count, len, clinks;
	int flag = 0, stat;
	Node *tmp, *buf;
	Link *links;

	links = malloc(10 * sizeof(Link));

	if (links == NULL) {
		fprintf(stderr, "increase() failed: %s\n", strerror(errno));
		return NULL;
	}

	while (TRUE) {
		stat = my_pthread_spin_lock(&s->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "decrease() failed: %s\n", strerror(stat));
			return NULL;
		}

		tmp = s->first;
		
		stat = my_pthread_spin_unlock(&s->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "decrease() failed: %s\n", strerror(stat));
			return NULL;
		}

		clinks = count = 0;
		len = strlen(tmp->value);

		links[clinks].len = len;
		links[clinks].count = 1;
		++clinks;

		stat = my_pthread_spin_lock(&tmp->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "decrease() failed: %s\n", strerror(stat));
			return NULL;
		}

		buf = tmp->next;
		stat = my_pthread_spin_unlock(&tmp->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "decrease() failed: %s\n", strerror(stat));
			return NULL;
		}

		tmp = buf;

		while (tmp != NULL) {
			len = strlen(tmp->value);

			for (int i = 0; i < clinks; ++i) {
				if (links[i].len > len) {
					count += links[i].count;
				}

				if (links[i].len == len) {
					++links[i].count;
					flag = 1;
				}
			}

			if (flag == 0) {
				links[clinks].len = len;
				links[clinks].count = 1;
				++clinks;
			}

			flag = 0;

			stat = my_pthread_spin_lock(&tmp->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "decrease() failed: %s\n", strerror(stat));
				return NULL;
			}

			buf = tmp->next;
			
			stat = my_pthread_spin_unlock(&tmp->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "decrease() failed: %s\n", strerror(stat));
				return NULL;
			}

			tmp = buf;
		}

		stat = my_pthread_spin_lock(&decrease_sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "decrease() failed: %s\n", strerror(stat));
			return NULL;
		}

		++decrease_v;
		decrease_c = count;
		
		stat = my_pthread_spin_unlock(&decrease_sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "decrease() failed: %s\n", strerror(stat));
			return NULL;
		}
	}

	free(links);
}

void* equal(void* arg) {
	Storage* s = (Storage*)arg;
	int count, len, clinks;
	int flag = 0, stat;
	Node *tmp, *buf;
	Link *links;

	links = malloc(10 * sizeof(Link));

	if (links == NULL) {
		fprintf(stderr, "increase() failed: %s\n", strerror(errno));
		return NULL;
	}

	while (TRUE) {
		stat = my_pthread_spin_lock(&s->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "equal() failed: %s\n", strerror(stat));
			return NULL;
		}

		tmp = s->first;
		
		stat = my_pthread_spin_unlock(&s->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "equal() failed: %s\n", strerror(stat));
			return NULL;
		}

		clinks = count = 0;
		len = strlen(tmp->value);

		links[clinks].len = len;
		links[clinks].count = 1;
		++clinks;

		stat = my_pthread_spin_lock(&tmp->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "equal() failed: %s\n", strerror(stat));
			return NULL;
		}

		buf = tmp->next;
		
		stat = my_pthread_spin_unlock(&tmp->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "equal() failed: %s\n", strerror(stat));
			return NULL;
		}

		tmp = buf;

		while (tmp != NULL) {
			len = strlen(tmp->value);

			for (int i = 0; i < clinks; ++i) {
				if (links[i].len == len) {
					count += links[i].count;
					++links[i].count;
					flag = 1;
				}
			}

			if (flag == 0) {
				links[clinks].len = len;
				links[clinks].count = 1;
				++clinks;
			}

			flag = 0;

			stat = my_pthread_spin_lock(&tmp->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "equal() failed: %s\n", strerror(stat));
				return NULL;
			}

			buf = tmp->next;
			
			stat = my_pthread_spin_unlock(&tmp->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "equal() failed: %s\n", strerror(stat));
				return NULL;
			}

			tmp = buf;
		}

		stat = my_pthread_spin_lock(&equal_sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "equal() failed: %s\n", strerror(stat));
			return NULL;
		}

		++equal_v;
		equal_c = count;
		
		stat = my_pthread_spin_unlock(&equal_sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "equal() failed: %s\n", strerror(stat));
			return NULL;
		}
	}

	free(links);
}

void* swap(void* arg) {
	Node *tmp, *next, *buf, *prew;
	Storage* s = (Storage*)arg;
	int choose = 0, stat;
	int flag = 1;

	while (TRUE) {
		choose = rand() % 2;

		if (choose == TRUE) {
			stat = my_pthread_spin_lock(&s->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap() failed: %s\n", strerror(stat));
				return NULL;
			}

			tmp = s->first;
			
			stat = my_pthread_spin_lock(&tmp->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap() failed: %s\n", strerror(stat));
				stat = my_pthread_spin_unlock(&s->sync);
				return NULL;
			}

			next = tmp->next;
			
			stat = my_pthread_spin_lock(&next->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap() failed: %s\n", strerror(stat));
				stat = my_pthread_spin_unlock(&tmp->sync);
				stat = my_pthread_spin_unlock(&s->sync);
				return NULL;
			}

			buf = next->next;

			s->first = next;
			next->next = tmp;
			tmp->next = buf;

			stat = my_pthread_spin_unlock(&next->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap() failed: %s\n", strerror(stat));
				stat = my_pthread_spin_unlock(&tmp->sync);
				stat = my_pthread_spin_unlock(&s->sync);
				return NULL;
			}

			stat = my_pthread_spin_unlock(&tmp->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap() failed: %s\n", strerror(stat));
				stat = my_pthread_spin_unlock(&s->sync);
				return NULL;
			}

			stat = my_pthread_spin_unlock(&s->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap() failed: %s\n", strerror(stat));
				return NULL;
			}

			stat = my_pthread_spin_lock(&swap_sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap() failed: %s\n", strerror(stat));
				return NULL;
			}

			swap_v++;
			
			stat = my_pthread_spin_unlock(&swap_sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap() failed: %s\n", strerror(stat));
				return NULL;
			}
		}

		stat = my_pthread_spin_lock(&s->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "swap() failed: %s\n", strerror(stat));
			return NULL;
		}


		prew = s->first;
		
		stat = my_pthread_spin_unlock(&s->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "swap() failed: %s\n", strerror(stat));
			return NULL;
		}

		while (flag == 1) {
			choose = rand() % 2;

			if (choose == TRUE) {
				stat = my_pthread_spin_lock(&prew->sync);

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap() failed: %s\n", strerror(stat));
					return NULL;
				}

				tmp = prew->next;

				if (tmp == NULL) {
					stat = my_pthread_spin_unlock(&prew->sync);

					if (stat != EXIT_SUCCESS) {
						fprintf(stderr, "swap() failed: %s\n", strerror(stat));
						return NULL;
					}

					break;
				}

				stat = my_pthread_spin_lock(&tmp->sync);

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap() failed: %s\n", strerror(stat));
					stat = my_pthread_spin_unlock(&prew->sync);
					return NULL;
				}

				next = tmp->next;

				if (next == NULL) {
					stat = my_pthread_spin_unlock(&tmp->sync);

					if (stat != EXIT_SUCCESS) {
						fprintf(stderr, "swap() failed: %s\n", strerror(stat));
						return NULL;
					}

					stat = my_pthread_spin_unlock(&prew->sync);

					if (stat != EXIT_SUCCESS) {
						fprintf(stderr, "swap() failed: %s\n", strerror(stat));
						return NULL;
					}

					break;
				}

				stat = my_pthread_spin_lock(&next->sync);

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap() failed: %s\n", strerror(stat));
					stat = my_pthread_spin_unlock(&tmp->sync);
					stat = my_pthread_spin_unlock(&prew->sync);
					return NULL;
				}

				buf = next->next;
				
				if (buf == NULL) {
					flag = 0;
				}

				prew->next = next;
				next->next = tmp;
				tmp->next = buf;

				stat = my_pthread_spin_unlock(&next->sync);

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap() failed: %s\n", strerror(stat));
					stat = my_pthread_spin_unlock(&tmp->sync);
					stat = my_pthread_spin_unlock(&prew->sync);
					return NULL;
				}
				
				stat = my_pthread_spin_unlock(&tmp->sync);

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap() failed: %s\n", strerror(stat));
					stat = my_pthread_spin_unlock(&prew->sync);
					return NULL;
				}

				stat = my_pthread_spin_unlock(&prew->sync);

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap() failed: %s\n", strerror(stat));
					return NULL;
				}

				stat = my_pthread_spin_lock(&swap_sync);

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap() failed: %s\n", strerror(stat));
					return NULL;
				}

				swap_v++;
				
				stat = my_pthread_spin_unlock(&swap_sync);

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap() failed: %s\n", strerror(stat));
					return NULL;
				}
			}

			stat = my_pthread_spin_lock(&prew->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap() failed: %s\n", strerror(stat));
				return NULL;
			}

			buf = prew->next;
			
			stat = my_pthread_spin_unlock(&prew->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap() failed: %s\n", strerror(stat));
				return NULL;
			}

			if (buf == NULL) {
				break;
			}

			prew = buf;
		}

		flag = 1;
	}
}

void init_storage(Storage* s) {
	int len = s->max_count;
	int part = len / 10;
	char str[100] = "test";

	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < part; ++j) {
			storage_add(s, str);
		}
		strcat(str, "test");
	}
}

int init_syncs() {
	int stat;

	stat = pthread_spin_init(&swap_sync, PTHREAD_PROCESS_PRIVATE);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_spin_init() failed: %s\n", strerror(stat));
		return SYNCS_ERROR;
	}

	stat = pthread_spin_init(&increase_sync, PTHREAD_PROCESS_PRIVATE);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_spin_init() failed: %s\n", strerror(stat));
		return SYNCS_ERROR;
	}

	stat = pthread_spin_init(&decrease_sync, PTHREAD_PROCESS_PRIVATE);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_spin_init() failed: %s\n", strerror(stat));
		return SYNCS_ERROR;
	}

	stat = pthread_spin_init(&equal_sync, PTHREAD_PROCESS_PRIVATE);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_spin_init() failed: %s\n", strerror(stat));
		return SYNCS_ERROR;
	}

	return EXIT_SUCCESS;
}

int main() {
	pthread_t inc_tid, dec_tid, eq_tid;
	pthread_t sw_tids[3];
	int stat;

	Storage* s = storage_init(100);

	if (s == NULL) {
		fprintf(stderr, "Cannot allocate memory for a storage\n");
		return MAIN_ERROR;
	}

	init_storage(s);

	stat = init_syncs();

	if (stat != EXIT_SUCCESS) {
		storage_destroy(s);
		return MAIN_ERROR;
	}

	stat = pthread_create(&inc_tid, NULL, increase, s);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_create() failed: %s\n", strerror(stat));
		storage_destroy(s);
		return MAIN_ERROR;
	}

	stat = pthread_create(&dec_tid, NULL, decrease, s);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_create() failed: %s\n", strerror(stat));
		storage_destroy(s);
		return MAIN_ERROR;
	}

	stat = pthread_create(&eq_tid, NULL, equal, s);

	if (stat != EXIT_SUCCESS) {
		fprintf(stderr, "pthread_create() failed: %s\n", strerror(stat));
		storage_destroy(s);
		return MAIN_ERROR;
	}

	for (int i = 0; i < 3; ++i) {
		stat = pthread_create(&sw_tids[i], NULL, swap, s);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "pthread_create() failed: %s\n", strerror(stat));
			storage_destroy(s);
			return MAIN_ERROR;
		}
	}

	while (TRUE) {
		show_storage(s);
		printf("\n");
		sleep(1);
	}

	return EXIT_SUCCESS;
}
