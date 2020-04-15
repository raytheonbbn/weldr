// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "pthread_mutex.h"

int fake_pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attrs) {
    if(mutex == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    //printf("Initializing mutex %p\n", mutex);
     
    mutex->__struct.waiting_count = 0;
    mutex->__struct.waiting_off = 0;

    mutex->__struct.owning_thread = MUTEX_NO_TID;
    mutex->__struct.recurs = 0;

    mutex->__struct.waiting_threads = calloc(MUTEX_MAX_WAITING_THREADS, sizeof(pthread_t));

    if(attrs != NULL) {
        mutex->__struct.attrs = *attrs;
    } else {
        mutex->__struct.attrs = 0; 
    }

    return 0; 
}

int fake_pthread_mutex_destroy(pthread_mutex_t *mutex) {
    free(mutex->__struct.waiting_threads);
    return 0;
}

int fake_pthread_mutex_lock(pthread_mutex_t *mutex) {
    pthread_t tid = fake_pthread_self();
    int ret = fake_pthread_mutex_trylock(mutex);

    //Check if we couldn't acquire the lock.
    if(ret == EBUSY) {
        //printf("Mutex %p is busy; blocking thread %u.\n", mutex, tid);
        errno = 0;
        ret = 0;

        //Check if the mutex is overloaded
        if(mutex->__struct.waiting_count >= MUTEX_MAX_WAITING_THREADS) {
            errno = EOVERFLOW;
            return EOVERFLOW;
        }

        //We need to wait.
        int idx = (mutex->__struct.waiting_off + mutex->__struct.waiting_count) % MUTEX_MAX_WAITING_THREADS;
        mutex->__struct.waiting_threads[idx] = tid;
        mutex->__struct.waiting_count++;

        pthread_block(tid);
    }

    //In all other cases, pass through ret.
    return ret;
}

int fake_pthread_mutex_trylock(pthread_mutex_t *mutex) {
    //printf("\n >>> in fake_pthread_mutex_trylock\n");
    if(mutex == NULL) {
        errno = EINVAL;
        return EINVAL;
    }

    //Mutex was initialized using the macro initializer.
    //Need to initialize it _our_ way.
    if(mutex->__struct.waiting_threads == NULL) {
        if(fake_pthread_mutex_init(mutex, NULL)) {
            return EINVAL;
        }
    }

    //Get the tid
    pthread_t tid = fake_pthread_self();
    //printf("Thread %u is trying mutex %p\n", tid, mutex);
    //printf("\towning_thread: %u\n", mutex->__struct.owning_thread);

    //Get the mutex type.
    int type = PTHREAD_MUTEX_DEFAULT;
    if(fake_pthread_mutexattr_gettype(&(mutex->__struct.attrs), &type)) {
        errno = EINVAL;
        return EINVAL;
    }

    //Check for deadlock cases
    if(mutex->__struct.owning_thread == tid) {
        if(type == PTHREAD_MUTEX_ERRORCHECK) {
            errno = EDEADLK;
            return EDEADLK;
        } else if(type == PTHREAD_MUTEX_RECURSIVE) {
            if(mutex->__struct.recurs >= MUTEX_MAX_RECURS) {
                errno = EAGAIN;
                return EAGAIN;
            } 
            mutex->__struct.recurs += 1;
            return 0;
        }
    }

    //Check if lock is unlocked.
    if(mutex->__struct.owning_thread == MUTEX_NO_TID) {
        //printf("\tMutex %p is unlocked.\n", mutex);
        mutex->__struct.owning_thread = tid;
        return 0;
    }

    //printf("\tMutex %p is busy.\n", mutex);
    //Lock isn't available.
    errno = EBUSY;
    return EBUSY;
}

int fake_pthread_mutex_unlock(pthread_mutex_t *mutex) {
    if(mutex == NULL) {
        errno = EINVAL;
        return EINVAL;
    }

    //Get the tid
    pthread_t tid = fake_pthread_self();
    
    //printf("Thread %u is unlocking mutex %p\n", tid, mutex);

    //Get the mutex type
    int type = PTHREAD_MUTEX_DEFAULT;
    if(fake_pthread_mutexattr_gettype(&(mutex->__struct.attrs), &type)) {
        errno = EINVAL;
        return EINVAL;
    }

    //Check for idiotic conditions.
    if(mutex->__struct.owning_thread != tid) {
        errno = EPERM;
        return EPERM;
    }

    //Check for recursive locks.
    if(type == PTHREAD_MUTEX_RECURSIVE) {
        mutex->__struct.recurs -= 1;
        if(mutex->__struct.recurs > 0) {
            return 0;
        }
    }

    //Check if there are waiting threads.
    //If threads are invalid, keep checking down the list.
    pthread_t new_tid;
    do {
        if(mutex->__struct.waiting_count == 0) {
            mutex->__struct.owning_thread = MUTEX_NO_TID;
            return 0;
        }

        new_tid = mutex->__struct.waiting_threads[mutex->__struct.waiting_off];
        //printf("\tThread %u is unlocking mutex %p; unblocking thread %u.\n", tid, mutex, new_tid);
        mutex->__struct.waiting_off = (mutex->__struct.waiting_off + 1) % MUTEX_MAX_WAITING_THREADS;
        mutex->__struct.waiting_count -= 1;

        mutex->__struct.owning_thread = new_tid;
    } while(pthread_wake(new_tid));

    return 0;
}

int fake_pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int new_ceiling, int *old_ceiling) {
    errno = ENOSYS;
    return ENOSYS;
}

int fake_pthread_mutex_getprioceiling(const pthread_mutex_t *mutex, int *curr_ceiling) {
    errno = ENOSYS;
    return ENOSYS;
}


