Функции:
- [`pthread_setaffinity_np`](https://linux.die.net/man/3/pthread_setaffinity_np)
- [`sched_yield`](https://linux.die.net/man/2/sched_yield)

При использовании множеством потоков разделяемого ресурса возникает либо ошибка инвалидного адреса, либо не соответствуют данные -- так как при разделении ресурсов необходима синхронизация доступа 
