#include "spinlock.h"

void spinlock_init(_Atomic spinlock_t* spin) {
    *spin = STAT_UNLOCK;
}

void spinlock_lock(_Atomic spinlock_t* spin) {
    while (ENDLESS) {
        int expected = STAT_UNLOCK;

        int stat = atomic_compare_exchange_strong(spin, &expected, STAT_LOCK);

        if (stat == true) {
            break;
        }
    }
}

void spinlock_unlock(_Atomic spinlock_t* spin) {
    int expected = STAT_LOCK;
    atomic_compare_exchange_strong(spin, &expected, STAT_UNLOCK);
}