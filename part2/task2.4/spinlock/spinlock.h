#include <stdatomic.h>
#include <stdbool.h>

#define ENDLESS 1

#define STAT_LOCK 0
#define STAT_UNLOCK 1

typedef _Atomic int spinlock_t;

void spinlock_init(_Atomic spinlock_t* spin);
void spinlock_lock(_Atomic spinlock_t* spin);
void spinlock_unlock(_Atomic spinlock_t* spin);
