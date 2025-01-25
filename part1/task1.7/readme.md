```c
void makecontext(ucontext_t *ucp, void (*func)(), int argc, ...);

int swapcontext(ucontext_t *restrict oucp, const ucontext_t *restrict ucp);
  // errors: ENOMEM Insufficient stack space left.

int getcontext(ucontext_t *ucp);

int setcontext(const ucontext_t *ucp);

typedef struct ucontext_t {
    struct ucontext_t *uc_link;
    sigset_t          uc_sigmask;
    stack_t           uc_stack;
    mcontext_t        uc_mcontext;
    ...
} ucontext_t;
```

Функции:
- [`makecontext`](https://linux.die.net/man/3/makecontext)
- [`swapcontext`](https://linux.die.net/man/3/makecontext)
- [`getcontext`](https://linux.die.net/man/3/getcontext)
- [`setcontext`](https://linux.die.net/man/3/getcontext)

