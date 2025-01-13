## Functions:

- int pthread_rwlock_destroy(pthread_rwlock_t **rwlock*);

- int pthread_rwlock_init(pthread_rwlock_t *restrict *rwlock*, const pthread_rwlockattr_t *restrict *attr*);

- pthread_rwlock_t *rwlock* = PTHREAD_RWLOCK_INITIALIZER;

- int pthread_rwlock_unlock(pthread_rwlock_t **rwlock*);

- int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock); 

- int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);

- int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);  

- int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);

## Errors:

The pthread_rwlock_init() function shall fail if:

- EAGAIN The system lacked the necessary resources (other than memory) to initialize another read-write lock.
- ENOMEM Insufficient memory exists to initialize the read-write lock.
- EPERM  The caller does not have the privilege to perform the operation.

These functions shall not return an error code of [EINTR].



The *pthread_rwlock_trywrlock*() function shall fail if:

**EBUSY**

The read-write lock could not be acquired for writing because it was already locked for reading or writing.

The *pthread_rwlock_trywrlock*() and *pthread_rwlock_wrlock*() functions may fail if:

**EINVAL**

The value specified by *rwlock* does not refer to an initialized read-write lock object.

The *pthread_rwlock_wrlock*() function may fail if:

**EDEADLK**

The current thread already owns the read-write lock for writing or reading.

These functions shall not return an error code of [EINTR].



The *pthread_rwlock_tryrdlock*() function shall fail if:

**EBUSY**

The read-write lock could not be acquired for reading because a writer holds the lock or a writer with the appropriate priority was blocked on it.

The *pthread_rwlock_rdlock*() and *pthread_rwlock_tryrdlock*() functions may fail if:

**EINVAL**

The value specified by *rwlock* does not refer to an initialized read-write lock object.

**EAGAIN**

The read lock could not be acquired because the maximum number of read locks for *rwlock* has been exceeded.

The *pthread_rwlock_rdlock*() function may fail if:

**EDEADLK**

The current thread already owns the read-write lock for writing.

These functions shall not return an error code of [EINTR].



If the Thread Execution Scheduling option is supported, and the threads involved in the lock are executing with the scheduling policies **SCHED_FIFO or SCHED_RR**, the calling thread shall not acquire the lock if a writer holds the lock or if writers of higher or equal priority are blocked on the lock; otherwise, the calling thread shall acquire the lock.

If the Threads Execution Scheduling option is supported, and the threads involved in the lock are executing with the **SCHED_SPORADIC** scheduling policy, the calling thread shall not acquire the lock if a writer holds the lock or if writers of higher or equal priority are blocked on the lock; otherwise, the calling thread shall acquire the lock.
