#include "compare.h"

void* increase(void* arg) {
	Storage* s = (Storage*)arg;
	Node *tmp, *tmp1, *buf;
	int count, stat;

    stat = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "increase: pthread_setcancelstate() id 0 failed: %s\n", strerror(stat));
        return NULL;
    }

	while (TRUE) {
		tmp = s->first;
        count = 0;

        while (TRUE) {
            stat = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "increase: pthread_setcancelstate() id 1 failed: %s\n", strerror(stat));
                return NULL;
            }

            pthread_testcancel();

            stat = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "increase: pthread_setcancelstate() id 2 failed: %s\n", strerror(stat));
                return NULL;
            }

            if (tmp == NULL) {
                break;
            }

            stat = my_pthread_mutex_lock(&tmp->sync);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "increase: my_pthread_mutex_lock() id 1 failed: %s\n", strerror(stat));
                return NULL;
            }

            tmp1 = tmp->next;

            if (tmp1 == NULL) {
                stat = my_pthread_mutex_unlock(&tmp->sync);

                if (stat != EXIT_SUCCESS) {
                    fprintf(stderr, "increase: my_pthread_mutex_unlock id 1 failed: %s\n", strerror(stat));
                    return NULL;
                }

                break;
            }

            stat = my_pthread_mutex_lock(&tmp1->sync);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "increase: my_pthread_mutex_lock id 2 failed: %s\n", strerror(stat));
                my_pthread_mutex_unlock(&tmp->sync);
                return NULL;
            }

            if (tmp->len > tmp1->len) {
                ++count;
            }

            buf = tmp;
            tmp = tmp1;

            stat = my_pthread_mutex_unlock(&buf->sync);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "increase: my_pthread_mutex_unlock() id 2 failed: %s\n", strerror(stat));
                my_pthread_mutex_unlock(&tmp->sync);
                return NULL;
            }

            stat = my_pthread_mutex_unlock(&tmp->sync);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "increase: my_pthread_mutex_unlock id 3 failed: %s\n", strerror(stat));
                return NULL;
            }
        }

		stat = change_increase(count);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "increase: change_increase id 1 failed: %s\n", strerror(stat));
			return NULL;
		}
	}
}

void* decrease(void* arg) {
	Storage* s = (Storage*)arg;
	Node *tmp, *tmp1, *buf;
	int count, stat;

    stat = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "decrease: pthread_setcancelstate id 0 failed: %s\n", strerror(stat));
        return NULL;
    }

	while (TRUE) {
		tmp = s->first;
        count = 0;

        while (TRUE) {
            stat = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "decrease: pthread_setcancelstate() id 1 failed: %s\n", strerror(stat));
                return NULL;
            }

            pthread_testcancel();

            stat = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "decrease: pthread_setcancelstate() id 2 failed: %s\n", strerror(stat));
                return NULL;
            }

            if (tmp == NULL) {
                break;
            }

            stat = my_pthread_mutex_lock(&tmp->sync);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "decrease: my_pthread_mutex_lock() id 1 failed: %s\n", strerror(stat));
                return NULL;
            }

            tmp1 = tmp->next;

            if (tmp1 == NULL) {
                stat = my_pthread_mutex_unlock(&tmp->sync);

                if (stat != EXIT_SUCCESS) {
                    fprintf(stderr, "decrease: my_pthread_mutex_unlock() id 1 failed: %s\n", strerror(stat));
                    return NULL;
                }

                break;
            }

            stat = my_pthread_mutex_lock(&tmp1->sync);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "decrease: my_pthread_mutex_lock() id 2 failed: %s\n", strerror(stat));
                my_pthread_mutex_unlock(&tmp->sync);
                return NULL;
            }

            if (tmp->len < tmp1->len) {
                ++count;
            }

            buf = tmp;
            tmp = tmp1;

            stat = my_pthread_mutex_unlock(&buf->sync);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "decrease: my_pthread_mutex_unlock() id 2 failed: %s\n", strerror(stat));
                my_pthread_mutex_unlock(&tmp->sync);
                return NULL;
            }

            stat = my_pthread_mutex_unlock(&tmp->sync);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "decrease: my_pthread_mutex_unlock() id 3 failed: %s\n", strerror(stat));
                return NULL;
            }
        }

		stat = change_decrease(count);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "decrease: change_decrease() id 1 failed: %s\n", strerror(stat));
			return NULL;
		}
	}
}

void* equal(void* arg) {
	Storage* s = (Storage*)arg;
	Node *tmp, *tmp1, *buf;
	int count, stat;

    stat = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    if (stat != EXIT_SUCCESS) {
        fprintf(stderr, "equal: pthread_setcancelstate() id 0 failed: %s\n", strerror(stat));
        return NULL;
    }

	while (TRUE) {
		tmp = s->first;
        count = 0;

        while (TRUE) {
            stat = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "equal: pthread_setcancelstate() id 1 failed: %s\n", strerror(stat));
                return NULL;
            }

            pthread_testcancel();

            stat = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "equal: pthread_setcancelstate() id 2 failed: %s\n", strerror(stat));
                return NULL;
            }

            if (tmp == NULL) {
                break;
            }

            stat = my_pthread_mutex_lock(&tmp->sync);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "equal: my_pthread_mutex_lock() id 1 failed: %s\n", strerror(stat));
                return NULL;
            }

            tmp1 = tmp->next;

            if (tmp1 == NULL) {
                stat = my_pthread_mutex_unlock(&tmp->sync);

                if (stat != EXIT_SUCCESS) {
                    fprintf(stderr, "equal: my_pthread_mutex_unlock() id 1 failed: %s\n", strerror(stat));
                    return NULL;
                }

                break;
            }

            stat = my_pthread_mutex_lock(&tmp1->sync);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "equal: my_pthread_mutex_lock() id 2 failed: %s\n", strerror(stat));
                my_pthread_mutex_unlock(&tmp->sync);
                return NULL;
            }

            if (tmp->len == tmp1->len) {
                ++count;
            }

            buf = tmp;
            tmp = tmp1;

            stat = my_pthread_mutex_unlock(&buf->sync);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "equal: my_pthread_mutex_unlock() id 2 failed: %s\n", strerror(stat));
                my_pthread_mutex_unlock(&tmp->sync);
                return NULL;
            }

            stat = my_pthread_mutex_unlock(&tmp->sync);

            if (stat != EXIT_SUCCESS) {
                fprintf(stderr, "equal: my_pthread_mutex_unlock() id 3 failed: %s\n", strerror(stat));
                return NULL;
            }
        }

		stat = change_equal(count);

		if (stat != EXIT_SUCCESS) {
			fprintf(stderr, "equal: change_equal() id 1 failed: %s\n", strerror(stat));
			return NULL;
		}
	}
}
