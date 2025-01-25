Функции:
- [`futex`](https://linux.die.net/man/2/futex)
- [`cas`](https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange)

```c
long futex(uint32_t *uaddr, int futex_op, uint32_t val, const struct timespec *timeout, uint32_t *uaddr2, uint32_t val3);
```

Целью сравнения с ожидаемым значением является предотвращение потерянных пробуждений. Если другой поток изменил значение слова futex после того, как вызывающий поток решил заблокироваться на основе предыдущего значения, и если другой поток выполнил операцию `FUTEX_WAKE` (или аналогичное пробуждение) после изменения значения и до этой операции `FUTEX_WAIT`, то вызывающий поток заметит изменение значения и не начнет спать.

при `explicit` задаются явные `memory_order` при успехе или неудаче

