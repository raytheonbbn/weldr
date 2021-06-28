// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#ifndef __PTHREAD_COND_H__
#define __PTHREAD_COND_H__
#include "pthread_shared.h"
#include "pthread_condattr.h"
#include "pthread_mutex.h"
#include <errno.h>
#include <time.h>

#define PTHREAD_COND_SIZE 48

#define COND_MAX_WAITING_THREADS 32

struct pthread_cond_struct {
    int waiting_count;
    int waiting_off;
    pthread_t *waiting_threads;

    pthread_condattr_t attrs;
};

union pthread_cond_u {
    struct pthread_cond_struct __struct;
    char __size[PTHREAD_COND_SIZE];
};

typedef union pthread_cond_u pthread_cond_t;

//Create/destroy the condition variable.
int   fake_pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attrs);
int   fake_pthread_cond_destroy(pthread_cond_t *cond);

//Unblock the first thread waiting on this variable.
int   fake_pthread_cond_signal(pthread_cond_t *cond);
//Unblock all threads waiting on this variable.
int   fake_pthread_cond_broadcast(pthread_cond_t *cond);

//Block on this condition variable until signaled.
//The thread must own the mutex passed to this function;
//once the thread is signaled, it will return from this function
//owning the mutex once more.
int   fake_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

//Block on this condition variable until signaled,
//or until the specified amount of time has passed.
int   fake_pthread_cond_timedwait(pthread_cond_t *cond,
          pthread_mutex_t *mutex, const struct timespec *timeout);

//Design:
//
//This is (yet another) queue of threads waiting to be woken.

#endif
