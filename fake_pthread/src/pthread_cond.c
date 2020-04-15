// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "pthread_cond.h"

int   fake_pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attrs) {
    //printf("\n >>> in fake_pthread_cond_init\n\n");
    if(cond == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    //printf("Initializing cond %p\n", cond);
    cond->__struct.waiting_count = 0;
    cond->__struct.waiting_off = 0;

    cond->__struct.waiting_threads = calloc(COND_MAX_WAITING_THREADS, sizeof(pthread_t));

    if(attrs != NULL) {
        cond->__struct.attrs = *attrs;
    } else {
        cond->__struct.attrs = 0;
    }
    return 0;
}

int   fake_pthread_cond_destroy(pthread_cond_t *cond) {
    //printf("\n >>> in fake_pthread_cond_destroy\n\n");
    free(cond->__struct.waiting_threads);
    return 0;
}

int   fake_pthread_cond_signal(pthread_cond_t *cond) {
    //printf("\n >>> in fake_pthread_cond_signal\n\n");
    if(cond == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    if(cond->__struct.waiting_threads == NULL) {
        if(fake_pthread_cond_init(cond, NULL)) {
            return EINVAL;
        }
    }
    pthread_t self = fake_pthread_self();
    //printf("Thread %u is signaling cond %p\n", self, cond);

    pthread_t tid;
    int ret = 0;
    do {
        if(ret) {
            perror("\tFailed to wake the last thread");
        }
        //Check if no one's waiting;
        if(cond->__struct.waiting_count == 0) {
            //printf("\tNo one left to wake.\n");
            break;
        }
        //If someone is waiting, unblock them.
        tid = cond->__struct.waiting_threads[cond->__struct.waiting_off];
        //printf("\tWaking %u\n", tid);
        cond->__struct.waiting_off = (cond->__struct.waiting_off + 1) % COND_MAX_WAITING_THREADS;
        cond->__struct.waiting_count -= 1;

    //If unblocking fails, try someone else.
    } while(ret = pthread_wake(tid));
    return 0;
}

int   fake_pthread_cond_broadcast(pthread_cond_t *cond) {
    //printf("\n >>> in fake_pthread_cond_broadcast\n\n");
    if(cond == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    if(cond->__struct.waiting_threads == NULL) {
        if(fake_pthread_cond_init(cond, NULL)) {
            return EINVAL;
        }
    }
    pthread_t self = fake_pthread_self();
    //printf("Thread %u is broadcasting cond %p\n", self, cond);

    while(cond->__struct.waiting_count != 0) {
        //Iterate over each blocked tid, and unblock it.
        pthread_t tid = cond->__struct.waiting_threads[cond->__struct.waiting_off];
        cond->__struct.waiting_off = (cond->__struct.waiting_off + 1) % COND_MAX_WAITING_THREADS;
        cond->__struct.waiting_count -= 1;
        pthread_wake(tid);
    }
    return 0;
}

int   fake_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    //printf("\n >>> in fake_pthread_cond_wait\n\n");
    if(cond == NULL || mutex == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    if(cond->__struct.waiting_threads == NULL) {
        if(fake_pthread_cond_init(cond, NULL)) {
            return EINVAL;
        }
    }

    pthread_t tid = fake_pthread_self();
    //printf("Thread %u is waiting on cond %p with mutex %p\n", tid, cond, mutex);

    //Check if the cond's queue is overloaded.
    if(cond->__struct.waiting_count >= COND_MAX_WAITING_THREADS) {
        errno = EOVERFLOW;
        return EOVERFLOW;
    }

    //Try to unlock the mutex.
    int ret;
    if((ret = fake_pthread_mutex_unlock(mutex))) {
        errno = ret;
        return ret;
    }

    //Put yourself in the waiting queue.
    int idx = (cond->__struct.waiting_off + cond->__struct.waiting_count) % COND_MAX_WAITING_THREADS;
    cond->__struct.waiting_threads[idx] = tid;
    cond->__struct.waiting_count += 1;

    //Block until someone wakes you up.
    pthread_block(tid);

    //Once woken, acquire the mitex.
    if((ret = fake_pthread_mutex_lock(mutex))) {
        errno = ret;
        return ret;
    }
    return 0;

}

int   fake_pthread_cond_timedwait(pthread_cond_t *cond,
          pthread_mutex_t *mutex, const struct timespec *timeout) {
    //TODO: This isn't strictly accurate
    //but our model isn't truly real-time.
    //printf("\n >>> in fake_pthread_cond_timedwait()\n\n");
    return fake_pthread_cond_wait(cond, mutex);
}
