### `Spin`
- `int pthread_spin_init(pthread_spinlock_t *lock, int pshared);`
  - `pshared`:
    1) `PTHREAD_PROCESS_PRIVATE`
    2) `PTHREAD_PROCESS_SHARED`
  - errors:
    1) `EAGAIN` -- The system has insufficient resources to initialize a new spin lock.
    2) `ENOMEM` -- Insufficient memory to initialize the spin lock.

- `int pthread_spin_destroy(pthread_spinlock_t *lock);`

- `int pthread_spin_lock(pthread_spinlock_t *lock);`
  - errors:
    1) `EDEADLOCK` -- The system detected a deadlock condition.

- `int pthread_spin_trylock(pthread_spinlock_t *lock);`
  - errors:
    1) `EBUSY` -- The spin lock is currently locked by another thread.

- `int pthread_spin_unlock(pthread_spinlock_t *lock);`

### `Mutex`
- `int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr);`
  - errors:
    1) `EAGAIN` -- The system lacked the necessary resources (other than memory) to initialize another `mutex`.
    2) `ENOMEM` -- Insufficient memory exists to initialize the `mutex`.
    3) `EPERM` -- The caller does not have the privilege to perform the operation.
    4) `EINVAL` -- The attributes object referenced by `attr` has the robust `mutex` attribute set without the process-shared attribute being set.
  - `attr`:
    - types:
      1) `NORMAL` (`DEF`)
      2) `ERRORCHECK`
      3) `RECURSIVE`
    - robustness (what happens when you acquire a `mutex` and the original owner died while possessing it)
    - process-shared attribute (for sharing a `mutex` across process boundaries)
    - protocol (how a thread behaves in terms of priority when a higher-priority thread wants the `mutex`)
    - priority ceiling (the priority at which the critical section will run, a way of preventing priority inversion)

- `int pthread_mutex_destroy(pthread_mutex_t *mutex);`

- `int pthread_mutex_lock(pthread_mutex_t *mutex);`

- `int pthread_mutex_trylock(pthread_mutex_t *mutex);`

- `int pthread_mutex_unlock(pthread_mutex_t *mutex);`
  - errors:
    1) `EPERM` -- The `mutex` type is `PTHREAD_MUTEX_ERRORCHECK` or `PTHREAD_MUTEX_RECURSIVE`, or the `mutex` is a robust `mutex`, and the current thread does not own the `mutex`.

### `Cond`
- `int pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr);`
  - errors: 
    1) `EAGAIN` -- The system lacked the necessary resources (other than memory) to initialize another condition variable.
    2) `ENOMEM` -- Insufficient memory exists to initialize the condition variable.

- `int pthread_cond_destroy(pthread_cond_t *cond);`
  - errors:
    1) `EBUSY` --  Some threads are currently waiting on `cond`.

- `int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex);`

- `int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);`
  - errors:
    1) `ETIMEDOUT` -- The condition variable was not signaled until the timeout specified by `abstime`.
    2) `EINTR` -- `pthread_cond_timedwait` was interrupted by a signal.

- `int pthread_cond_signal(pthread_cond_t *cond);`
  - errors: These functions shall not return an error code of `[EINTR]`.

- `int pthread_cond_broadcast(pthread_cond_t *cond);`
  - errors: These functions shall not return an error code of `[EINTR]`.

### `Sem`
- `int sem_init(sem_t *sem, int pshared, unsigned int value);`
  - errors:
    1) `EINVAL` -- value exceeds `SEM_VALUE_MAX`.
    2) `ENOSYS` -- `pshared` is nonzero, but the system does not support process-shared semaphores.

- `int sem_destroy(sem_t *sem);`
  - errors: 
    1) `EINVAL` -- `sem` is not a valid semaphore.

- `int sem_wait(sem_t *sem);`

- `int sem_trywait(sem_t *sem);`

- `int sem_timedwait(sem_t *restrict sem, const struct timespec *restrict abs_timeout);`

- `int sem_post(sem_t *sem);`
  - errors:
    1) `EINVAL` -- `sem` is not a valid semaphore.
    2) `EOVERFLOW` -- The maximum allowable value for a semaphore would be exceeded.
