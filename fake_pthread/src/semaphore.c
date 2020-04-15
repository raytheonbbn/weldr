// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "semaphore_impl.h"

//TODO: Need global storage for named semaphores.

int fake_sem_init(sem_t *sem, int pshared, unsigned int value) {
    if(sem == NULL) {
        errno = EINVAL;
        return EINVAL;
    }

    // Case where semaphore is shared between threads of a process, pshared == 0
    // Case where semaphore is shared between processes, pshared != 0
    if(pshared) {
        errno = ENOSYS;
        return ENOSYS;
    }
    
    sem->value = value;
    sem->waiting_count = 0;
    sem->waiting_off = 0;
    sem->waiting_threads = calloc(SEM_MAX_WAITING_THREADS, sizeof(pthread_t));

    if(sem->waiting_threads == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    return 0;
}

int fake_sem_destroy(sem_t *sem) {
    if(sem == NULL) {
        errno = EINVAL;
        return EINVAL;
    }

    free(sem->waiting_threads);
    return 0;
}

sem_t *fake_sem_open(const char *name, int oflag, ...) {
    errno = ENOSYS;
    return NULL;
}

int fake_sem_close(sem_t *sem) {
    errno = ENOSYS;
    return ENOSYS; 
}

int fake_sem_unlink(const char *name) {
    errno = ENOSYS;
    return ENOSYS;
}

int fake_sem_trywait(sem_t *sem) {
    if(sem == NULL) {
        errno = EINVAL;
        return -1;
    }

    //Check the value.
    if(sem->value == 0) {
        errno = EAGAIN;
        return -1;
    } else {
        sem->value--;
        return 0;     
    }
}

int fake_sem_wait(sem_t *sem) {
    if(sem == NULL) {
        errno = EINVAL;
        return -1;
    }
    if(fake_sem_trywait(sem)) {
        int idx;
        pthread_t tid;
        while(!sem->value) {
            if(sem->waiting_count > SEM_MAX_WAITING_THREADS) {
                errno = ENOMEM;
                return -1;
            }
            idx = (sem->waiting_count + sem->waiting_off) % SEM_MAX_WAITING_THREADS;
            tid = fake_pthread_self();
            sem->waiting_threads[idx] = tid;
            sem->waiting_count++;
            pthread_block(tid);
        }
        // Once we have made it here, thread is unblocked so need to
        // decrement waiting_count and sem->value
        sem->waiting_count--;
        sem->value--;
    }
    return 0;
}

int fake_sem_post(sem_t *sem) {
    if(sem == NULL) {
        errno = EINVAL;
        return -1;
    }
    sem->value++;
    if(sem->value > 0) {
        pthread_t tid = sem->waiting_threads[sem->waiting_off];
        sem->waiting_off = (sem->waiting_off + 1) % SEM_MAX_WAITING_THREADS;
        pthread_wake(tid);
    }

    return 0;
}

int fake_sem_getvalue(sem_t *sem, int *cur_value) {
    if(sem == NULL) {
        errno = EINVAL;
        return -1;
    }

    *cur_value = sem->value;

    return 0;
}
