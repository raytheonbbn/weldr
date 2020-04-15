// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#ifndef __PTHREAD_STRUCT_H__
#define __PTHREAD_STRUCT_H__
#include "pthread_shared.h"
#include "pthread_cond.h"
#include "pthread_mutex.h"
#include <stdlib.h>
#include <setjmp.h>
#include <sys/mman.h>

#define PTHREAD_STATE_ACTIVE 0 //The thread is currently running.
#define PTHREAD_STATE_RUNNABLE 1 //The thread isn't running, but it could be scheduled.
#define PTHREAD_STATE_BLOCKED 2 //The thread isn't running, and something else has taken over scheduling it.
#define PTHREAD_STATE_ZOMBIE 3 //The thread has exited, and is waiting for cleanup.

struct pthread_struct {
    //Basic parameters
    int valid;  //Is this thread an actual thread, or just empty memory?
    pthread_t tid;  //The thread ID.
    int state;  //The current state of the thread.
    int detached;  //Whether or not this thread is detached.
    //Suspension Parameters
    jmp_buf hang_point;
    //Stack parameters
    void* stack_addr;
    size_t stack_size;
    //Cleanup functions
    int num_user_cleanup_routines;
    void (*user_cleanup_routines[MAX_CLEANUP_ROUTINES])(void *);
    void *user_cleanup_args[MAX_CLEANUP_ROUTINES];

    int num_system_cleanup_routines;
    void (*system_cleanup_routines[MAX_CLEANUP_ROUTINES])(void *);
    void *system_cleanup_args[MAX_CLEANUP_ROUTINES];
    //Cancel parameters
    int cancel_state;
    int cancel_type;
    int canceled;
    //Join parameters
    pthread_mutex_t join_mutex;
    pthread_cond_t join_cond;
    int join_registered;
    void *join_val;
};

#endif
