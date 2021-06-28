// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#ifndef __PTHREAD_MUTEX_H__
#define __PTHREAD_MUTEX_H__
#include "pthread_shared.h"
#include "pthread_control.h"
#include "pthread_thread.h"
#include "pthread_mutexattr.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

//Definitions for pthread_mutex.
#define MUTEX_STRUCT_SIZE 40

#define MUTEX_MAX_WAITING_THREADS 32
#define MUTEX_MAX_RECURS 0xFFFFFFFF

#define MUTEX_NO_TID -1

//Mutex struct
struct pthread_mutex_struct {
    int waiting_count;
    int waiting_off;

    pthread_t *waiting_threads;
    pthread_t owning_thread;

    int recurs;

    pthread_mutexattr_t attrs;
};

//Rename mutex struct to not need the "struct" prefix.
union pthread_mutex_u {
    struct pthread_mutex_struct __struct;
    char __size[MUTEX_STRUCT_SIZE];
};

typedef union pthread_mutex_u pthread_mutex_t;

//Initialize a mutex with a given set of attributes.
//Initializing a mutex twice is bad.
int   fake_pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attrs);

//Destroy a mutex.
//This is only safe to do if the mutex is initialized and unlocked.
int   fake_pthread_mutex_destroy(pthread_mutex_t *mutex);

//Aquire a lock on a mutex.
//The thread will block until the mutex becomes available.
//How this behaves in the case of multiple calls from the same
//thread depends on how the mutex was initialized;
//see mutex_attrs.h for details.
//Returns 0 if the lock was aquired, otherwise an error number is returned.
//Trying this on an uninitialized mutex is bad.
int   fake_pthread_mutex_lock(pthread_mutex_t *mutex);

//Check if a mutex is available for locking.
//This has the same behavior as fake_pthread_mutex_lock,
//except it returns an error immediately if the mutex
//is already locked.
//Trying this on an uninitalized mutex is bad.
int   fake_pthread_mutex_trylock(pthread_mutex_t *mutex);

//Unlock a locked mutex.
//Exactly what this does to blocked threads
//depends on the mutex attributes.  See pthread_mutexattr.h.
//Trying this on an uninitalized mutex or one you didn't lock is bad.
int   fake_pthread_mutex_unlock(pthread_mutex_t *mutex);

//Get/set a mutex's priority ceiling.
//Under certain mutex modes, mutexes use the priority ceiling
//to control which threads can access the critical section;
//a thread with too high a priority number will be denied
//access.
int   fake_pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int new_ceiling, int *old_ceiling);
int   fake_pthread_mutex_getprioceiling(const pthread_mutex_t *mutex, int *curr_ceiling);

//Design:
//
//This might actually be easy to implement;
//a mutex is essentially a queue of threads
//waiting for the critical section to become
//available.  The mutex doesn't need to know
//how threads are put to sleep and reawoken;
//it only needs to know when to do each.
//
//This is actually even easier than a truly-thread-safe mutex,
//since the mutex operations themselves will never get preempted.

#endif
