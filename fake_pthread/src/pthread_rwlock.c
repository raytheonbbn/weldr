// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "pthread_rwlock.h"

int fake_pthread_rwlock_init(pthread_rwlock_t *lock, const pthread_rwlockattr_t *attrs) {
    if(lock == NULL) {
        errno = EINVAL;
        return EINVAL;
    } 
    //printf("Initializing rwlock %p\n", lock);
    lock->__struct.waiting_count = 0;
    lock->__struct.waiting_off = 0;
    lock->__struct.writing_thread = 0;

    lock->__struct.waiting_threads = calloc(RWLOCK_MAX_WAITING_THREADS, sizeof(pthread_t));
    lock->__struct.waiting_types = calloc(RWLOCK_MAX_WAITING_THREADS, sizeof(char));
    //printf("\tthreads: %p\n", lock->__struct.waiting_threads);
    //printf("\ttypes: %p\n", lock->__struct.waiting_types);

    lock->__struct.attrs = (pthread_rwlockattr_t*)attrs;
    return 0;
}

int fake_pthread_rwlock_destroy(pthread_rwlock_t *lock) {
    free(lock->__struct.waiting_threads);
    free(lock->__struct.waiting_types);
    return 0;
}

int fake_pthread_rwlock_tryrdlock(pthread_rwlock_t *lock) {
    if(lock == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    if(lock->__struct.waiting_threads == NULL) {
        if(fake_pthread_rwlock_init(lock, NULL)) {
            return EINVAL;
        }
    }

    pthread_t tid = fake_pthread_self();
    //printf("Thread %lu trying read lock on rwlock %p\n", tid, lock);
    
    if(tid == lock->__struct.writing_thread) {
        errno = EDEADLK;
        return EDEADLK;
    }

    if(lock->__struct.writing_thread != 0 || lock->__struct.waiting_types[lock->__struct.waiting_off] == RWLOCK_WAITING_WRITE) {
        errno = EBUSY;
        return EBUSY;
    }

    if(lock->__struct.reading_count >= RWLOCK_MAX_READING_THREADS) {
        errno = EAGAIN;
        return EAGAIN;
    } 

    lock->__struct.reading_count++;    
    return 0;
}

int fake_pthread_rwlock_rdlock(pthread_rwlock_t *lock) {
    int ret = fake_pthread_rwlock_tryrdlock(lock);
    if(ret == EBUSY) {
        pthread_t tid = fake_pthread_self();
        //printf("Thread %lu read-blocking on rwlock %p\n", tid, lock);
        if(lock->__struct.waiting_count >= RWLOCK_MAX_WAITING_THREADS) {
            errno = EAGAIN;
            return EAGAIN;
        }

        int idx = (lock->__struct.waiting_off + lock->__struct.waiting_count) % RWLOCK_MAX_WAITING_THREADS;
        lock->__struct.waiting_threads[idx] = tid;
        lock->__struct.waiting_types[idx] = RWLOCK_WAITING_READ;
        lock->__struct.waiting_count++;
        pthread_block(fake_pthread_self());
    }
    return 0;
}

int fake_pthread_rwlock_trywrlock(pthread_rwlock_t *lock) {
    if(lock == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    if(lock->__struct.waiting_threads == NULL) {
        if(fake_pthread_rwlock_init(lock, NULL)) {
            return EINVAL;
        }
    }
    
    pthread_t tid = fake_pthread_self();
    //printf("Thread %lu trying write lock on rwlock %p\n", tid, lock);
    
    //Check if we've already locked this lock.
    //This won't trigger if we're far up the queue,
    //but we will get an EBUSY in that case.
    if(tid == lock->__struct.writing_thread) {
        errno = EDEADLK;
        return EDEADLK;
    }

    if(lock->__struct.reading_count > 0) {
        errno = EBUSY;
        return EBUSY;
    }

    if(lock->__struct.writing_thread != 0 || lock->__struct.waiting_types[lock->__struct.waiting_off] == RWLOCK_WAITING_WRITE) {
        errno = EBUSY;
        return EBUSY;
    }

    lock->__struct.writing_thread = tid;
    return 0;
}

int fake_pthread_rwlock_wrlock(pthread_rwlock_t *lock) {
    int ret = fake_pthread_rwlock_trywrlock(lock);
    if(ret == EBUSY) {
        if(lock->__struct.waiting_count >= RWLOCK_MAX_WAITING_THREADS) {
            errno = EAGAIN;
            return EAGAIN;
        }

        pthread_t tid = fake_pthread_self();
        //printf("Thread %lu write-blocking on rwlock %p\n", tid, lock);
        int idx = (lock->__struct.waiting_off + lock->__struct.waiting_count) % RWLOCK_MAX_WAITING_THREADS;
        lock->__struct.waiting_threads[idx] = tid;
        lock->__struct.waiting_types[idx] = RWLOCK_WAITING_WRITE;
        lock->__struct.waiting_count++;
        pthread_block(tid); 
    }
    return 0;
}

int fake_pthread_rwlock_unlock(pthread_rwlock_t *lock) {
    if(lock == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    pthread_t tid = fake_pthread_self();
    //printf("Thread %lu unlocking rwlock %p\n", tid, lock);
    if(lock->__struct.writing_thread == tid) {
        lock->__struct.writing_thread = 0;
    } else if(lock->__struct.reading_count == 0) {
        errno = EPERM;
        return EPERM;
    } else if(lock->__struct.writing_thread != 0) {
        errno = EPERM;
        return EPERM;
    } else {
        lock->__struct.reading_count--;
    }

    int done = (lock->__struct.waiting_count <= 0);
    //printf("\n >>> UNLOCK waiting count before while = %d \n", lock->__struct.waiting_count);
    while(!done) { 
        int waiting_tid = lock->__struct.waiting_threads[lock->__struct.waiting_off];
        char waiting_type = lock->__struct.waiting_types[lock->__struct.waiting_off];
        int ok = 0;
        int is_writer = 0;
        if(waiting_type == RWLOCK_WAITING_READ) {
            //printf(" >>> UNLOCK tid %d waiting to READ \n", waiting_tid);
            lock->__struct.reading_count++;
            ok = 1;
        } else if(waiting_type == RWLOCK_WAITING_WRITE) {
            //printf(" >>> UNLOCK tid %d waiting to WRITE \n", waiting_tid);
            done = 1;
            is_writer = 1;
            
            if(lock->__struct.reading_count == 0) {
                //printf(" >>> UNLOCK no readers, setting ok = 1 \n");
                lock->__struct.writing_thread = waiting_tid;
                ok = 1;
            } else {
                //printf(" >>> UNLOCK readers exist, setting ok = 0 \n");
                ok = 0;
            }
        } else {
            //Should probably report some kind of error.  Just ignore.
            //printf(" >>> UNLOCK neither waiting to READ or WRITE, setting ok = 1 \n");
            ok = 1;
        }
        if(ok) {
            //printf(" >>> UNLOCK OK condition \n");
            lock->__struct.waiting_off = (lock->__struct.waiting_off + 1) % RWLOCK_MAX_WAITING_THREADS;
            lock->__struct.waiting_count -= 1;
            done = ((lock->__struct.waiting_count <= 0) || is_writer);
            //printf(" >>> UNLOCK waiting count = %d \n", lock->__struct.waiting_count);
            //printf(" >>> UNLOCK is_writer = %d \n", is_writer);
            //printf(" >>> value of done = %d\n", done);
            pthread_wake(waiting_tid);
        }
    }
    //printf(" >>> UNLOCK after while, returning...\n\n");
    return 0; // Added by MR, not here before
}
