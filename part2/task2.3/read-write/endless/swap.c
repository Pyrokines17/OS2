#include "swap.h"

void* swap(void* arg) {
	Node *tmp, *next, *buf, *prew;
	Storage* s = (Storage*)arg;
	int choose = 0, stat;
	int flag = 1;

	while (TRUE) {
		choose = rand() % 2;

		if (choose == TRUE) {
			stat = my_pthread_rwlock_wrlock(&s->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap: my_pthread_rwlock_wrlock() id 1 failed: %s\n", strerror(stat));
				return NULL;
			}

			tmp = s->first;
			stat = my_pthread_rwlock_wrlock(&tmp->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap: my_pthread_rwlock_wrlock() id 2 failed: %s\n", strerror(stat));
				my_pthread_rwlock_unlock(&s->sync);
				return NULL;
			}

			next = tmp->next;
			stat = my_pthread_rwlock_wrlock(&next->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap: my_pthread_rwlock_wrlock() id 3 failed: %s\n", strerror(stat));
				my_pthread_rwlock_unlock(&tmp->sync);
				my_pthread_rwlock_unlock(&s->sync);
				return NULL;
			}

			buf = next->next;

			s->first = next;
			next->next = tmp;
			tmp->next = buf;

			stat = my_pthread_rwlock_unlock(&next->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap: my_pthread_rwlock_unlock() id 1 failed: %s\n", strerror(stat));
				my_pthread_rwlock_unlock(&tmp->sync);
				my_pthread_rwlock_unlock(&s->sync);
				return NULL;
			}

			stat = my_pthread_rwlock_unlock(&tmp->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap: my_pthread_rwlock_unlock() id 2 failed: %s\n", strerror(stat));
				my_pthread_rwlock_unlock(&s->sync);
				return NULL;
			}

			stat = my_pthread_rwlock_unlock(&s->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap: my_pthread_rwlock_unlock() id 3 failed: %s\n", strerror(stat));
				return NULL;
			}

			stat = change_swap();

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap: change_swap() id 1 failed: %s\n", strerror(stat));
				return NULL;
			}
		}

		stat = my_pthread_rwlock_wrlock(&s->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "swap: my_pthread_rwlock_wrlock() id 4 failed: %s\n", strerror(stat));
			return NULL;
		}

		prew = s->first;
		stat = my_pthread_rwlock_unlock(&s->sync);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "swap: my_pthread_rwlock_unlock() id 4 failed: %s\n", strerror(stat));
			return NULL;
		}

		while (flag == 1) {
			choose = rand() % 2;

			if (choose == TRUE) {
				stat = my_pthread_rwlock_wrlock(&prew->sync);

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap: my_pthread_rwlock_wrlock() id 5 failed: %s\n", strerror(stat));
					return NULL;
				}

				tmp = prew->next;

				if (tmp == NULL) {
					stat = my_pthread_rwlock_unlock(&prew->sync);

                    if (stat != EXIT_SUCCESS) {
                        fprintf(stderr, "swap: my_pthread_rwlock_unlock() id 5 failed: %s\n", strerror(stat));
                        return NULL;
                    }

					break;
				}

				stat = my_pthread_rwlock_wrlock(&tmp->sync);

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap: my_pthread_rwlock_wrlock() id 6 failed: %s\n", strerror(stat));
					my_pthread_rwlock_unlock(&prew->sync);
					return NULL;
				}

				next = tmp->next;

				if (next == NULL) {
					stat = my_pthread_rwlock_unlock(&tmp->sync);

					if (stat != EXIT_SUCCESS) {
						fprintf(stderr, "swap: my_pthread_rwlock_unlock() id 6 failed: %s\n", strerror(stat));
						return NULL;
					}

					stat = my_pthread_rwlock_unlock(&prew->sync);

					if (stat != EXIT_SUCCESS) {
						fprintf(stderr, "swap: my_pthread_rwlock_unlock() id 7 failed: %s\n", strerror(stat));
						return NULL;
					}

					break;
				}

				stat = my_pthread_rwlock_wrlock(&next->sync);

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap: my_pthread_rwlock_wrlock() id 7 failed: %s\n", strerror(stat));
					my_pthread_rwlock_unlock(&tmp->sync);
					my_pthread_rwlock_unlock(&prew->sync);
					return NULL;
				}

				buf = next->next;
				
				if (buf == NULL) {
					flag = 0;
				}

				prew->next = next;
				next->next = tmp;
				tmp->next = buf;

				stat = my_pthread_rwlock_unlock(&next->sync);

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap: my_pthread_rwlock_unlock() id 8 failed: %s\n", strerror(stat));
					my_pthread_rwlock_unlock(&tmp->sync);
					my_pthread_rwlock_unlock(&prew->sync);
					return NULL;
				}
				
				stat = my_pthread_rwlock_unlock(&tmp->sync);

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap: my_pthread_rwlock_unlock() id 9 failed: %s\n", strerror(stat));
					my_pthread_rwlock_unlock(&prew->sync);
					return NULL;
				}

				stat = my_pthread_rwlock_unlock(&prew->sync);

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap: my_pthread_rwlock_unlock() id 10 failed: %s\n", strerror(stat));
					return NULL;
				}

				stat = change_swap();

				if (stat != EXIT_SUCCESS) {
					fprintf(stderr, "swap: change_swap() id 2 failed: %s\n", strerror(stat));
					return NULL;
				}
			}

			stat = my_pthread_rwlock_wrlock(&prew->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap: my_pthread_rwlock_wrlock() id 8 failed: %s\n", strerror(stat));
				return NULL;
			}

			buf = prew->next;
			stat = my_pthread_rwlock_unlock(&prew->sync);

			if (stat != EXIT_SUCCESS) {
				fprintf(stderr, "swap: my_pthread_rwlock_unlock() id 11 failed: %s\n", strerror(stat));
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
