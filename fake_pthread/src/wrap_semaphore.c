// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "semaphore_impl.h"

int  __real_sem_close(sem_t *);
int  __real_sem_destroy(sem_t *);
int  __real_sem_getvalue(sem_t *, int *);
int  __real_sem_init(sem_t *, int, unsigned int);
sem_t *__real_sem_open(const char *, int, ...);
int  __real_sem_post(sem_t *);
int  __real_sem_trywait(sem_t *);
int  __real_sem_unlink(const char *);
int  __real_sem_wait(sem_t *);

int  __wrap_sem_close(sem_t *sem) {
    return fake_sem_close(sem);
}

int  __wrap_sem_destroy(sem_t *sem) {
    return fake_sem_destroy(sem);
}

int  __wrap_sem_getvalue(sem_t *sem, int *val) {
    return fake_sem_getvalue(sem, val);
}

int  __wrap_sem_init(sem_t *sem, int pshared, unsigned int val) {
    return fake_sem_init(sem, pshared, val);
}

sem_t *__wrap_sem_open(const char *name, int oflag, ...) {
    //TODO: I hate varargs.
    errno = ENOSYS;
    return NULL;
}

int  __wrap_sem_post(sem_t *sem) {
    return fake_sem_post(sem);
}

int  __wrap_sem_trywait(sem_t *sem) {
    return fake_sem_trywait(sem);
}
int  __wrap_sem_unlink(const char *name) {
    return fake_sem_unlink(name);
}
int  __wrap_sem_wait(sem_t *sem) {
    return fake_sem_wait(sem);
}
