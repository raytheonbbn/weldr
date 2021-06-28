// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#ifndef __PTHREAD_RWLOCK_H__
#define __PTHREAD_RWLOCK_H__
#include "pthread_shared.h"
#include "pthread_control.h"
#include "pthread_thread.h"
#include "pthread_rwlockattr.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Definitions for pthread_rwlock

#define PTHREAD_RWLOCK_SIZE 56

#define RWLOCK_MAX_WAITING_THREADS 32
#define RWLOCK_MAX_READING_THREADS 0xFFFFFFFF

#define RWLOCK_WAITING_READ '\0'
#define RWLOCK_WAITING_WRITE '\1'

//Rwlock struct
struct pthread_rwlock_struct {
    unsigned int waiting_count;
    unsigned int waiting_off;

    unsigned int reading_count;
    
    pthread_t *waiting_threads;

    char *waiting_types;

    pthread_t writing_thread;

    pthread_rwlockattr_t *attrs;
};

union pthread_rwlock_u {
    struct pthread_rwlock_struct __struct;
    char __size[PTHREAD_RWLOCK_SIZE];
};

//Rename the rwlock struct to not need the "struct" prefix.
typedef union pthread_rwlock_u pthread_rwlock_t;

//Initalize a rwlock object.
//Initializing one twice is bad.
int   fake_pthread_rwlock_init(pthread_rwlock_t *lock,
          const pthread_rwlockattr_t *attrs);

//Destroy a rwlock
//Destroying an actively-held rwlock or a non-initalized one is bad.
int   fake_pthread_rwlock_destroy(pthread_rwlock_t *lock);

//Attempt to aquire the lock for reading.
//This will block if there's an active write lock.
//Trying to acquire a read lock while you have a write lock is bad.
int   fake_pthread_rwlock_rdlock(pthread_rwlock_t *lock);

//Check if the lock is available for reading.
//This has the same behavior as rdlock, except
//it immediately returns with an error if the lock
//is write-locked.
//Trying this on an uninitialized lock is a bad idea. 
int   fake_pthread_rwlock_tryrdlock(pthread_rwlock_t *lock);

//Attempt to acquire the lock for writing.
//This will block if there's another active read or write lock.
//Trying to lock a lock you've already locked
//is a bad idea.
int   fake_pthread_rwlock_wrlock(pthread_rwlock_t *lock);

//Check if the lock is available for writing.
//This behaves the same as wrlock, except it immediately
//returns with an error if the lock is read- or write-locked.
int   fake_pthread_rwlock_trywrlock(pthread_rwlock_t *lock);

//Unlock whatever lock the thread had on this lock.
//Attempting to unlock an uninitialized lock,
//or one you didn't lock yourself are bad ideas.
int   fake_pthread_rwlock_unlock(pthread_rwlock_t *lock);

//Design
//
//As with a mutex, this is a queue of threads.
//The only difference is that the rules for when
//a thread can get cleared from the queue differ
//between reader threads and writer threads.
//The lock doesn't need to know how threads get blocked
//or woken, only that they can be blocked and woken.
//
//As with mutexes, the implementation itself doesn't
//need to be thread-safe, since it's used in a non-preemptive
//environment.

#endif
