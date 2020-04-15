// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#ifndef __PTHREAD_THREAD_ATTR_H__
#define __PTHREAD_THREAD_ATTR_H__
#include "pthread_shared.h"
#include "sched.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

//sys/types.h links in the pthread type headers.  Not good.
#ifndef size_t
typedef unsigned long size_t;
#endif

struct pthread_attr_struct {
    int detach_state;
    size_t guard_size;
    int inherit_sched;
    struct sched_param *sparam;
    int sched_policy;
    int scope;
    void *stack_addr;
    size_t stack_size;
};

typedef struct pthread_attr_struct pthread_attr_t;

//Create/destroy a thread attribute object.
int   fake_pthread_attr_init(pthread_attr_t *attrs);
int   fake_pthread_attr_destroy(pthread_attr_t *attrs);

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

//Get/set the initial detach state of the thread.
//- PTHREAD_CREATE_JOINABLE (Default): The thread's memory is preserved on exit, allowing joins.
//- PTHREAD_CREATE_DETACHED: The thread is created detached; its memory will be cleaned on exit.
int   fake_pthread_attr_getdetachstate(const pthread_attr_t *attrs, int *cur_state);
int   fake_pthread_attr_setdetachstate(pthread_attr_t *attrs, int new_state);

//Get/set the guard size for this thread.
//Thread guards are unused padding between thread stacks;
//attempting to access these regions, via stack overflow or
//by accidentally stack-allocating a lot of memory, will cause a segfault.
//By default, the guard size is one page (whatever that is for your system).
int   fake_pthread_attr_getguardsize(const pthread_attr_t *attrs, size_t *cur_size);
int   fake_pthread_attr_setguardsize(pthread_attr_t *attrs, size_t new_size);

#define PTHREAD_INHERIT_SCHED 0
#define PTHREAD_EXPLICIT_SCHED 1

//Get/set the scheduler inheritance policy for this thread.
//- PTHREAD_INHERIT_SCHED(Default): Take the scheduler params from the parent thread.
//- PTHREAD_EXPLICIT_SCHED: Take the scheduler params from this struct.
int   fake_pthread_attr_getinheritsched(const pthread_attr_t *attrs, int *cur_inherit);
int   fake_pthread_attr_setinheritsched(pthread_attr_t *attrs, int new_inherit);

//Get/set the scheuler params for this thread.
int   fake_pthread_attr_getschedparam(const pthread_attr_t *attrs,
          struct sched_param *cur_param);
int   fake_pthread_attr_setschedparam(pthread_attr_t *attrs,
        const struct sched_param *new_param);

//Get/set the scheduler policy for this thread.
int   fake_pthread_attr_getschedpolicy(const pthread_attr_t *attrs, int *cur_policy);
int   fake_pthread_attr_setschedpolicy(pthread_attr_t *attrs, int new_policy);

#define PTHREAD_SCOPE_SYSTEM 0
#define PTHREAD_SCOPE_PROCESS 1

//Get/set the scope of this thread.
//This is another glorious feature of realtime systems.
int   fake_pthread_attr_getscope(const pthread_attr_t *attrs, int *cur_scope);
int   fake_pthread_attr_setscope(pthread_attr_t *attrs, int new_scope);

//Get/set the stack address for this thread.
//This defines the first address for the stack base pointer.
int   fake_pthread_attr_getstackaddr(const pthread_attr_t *attrs, void **cur_addr);
int   fake_pthread_attr_setstackaddr(pthread_attr_t *attrs, void *new_addr);

#define PTHREAD_STACK_MIN 65536

//Get/set the stack size for this thread.
//This defines the minimum size allowed for this thread's stack.
//This can't be less than PTHREAD_STACK_MIN or greater than system-defined limits.
int   fake_pthread_attr_getstacksize(const pthread_attr_t *attrs, size_t *cur_size);
int   fake_pthread_attr_setstacksize(pthread_attr_t *attrs, size_t new_size);


//Design
//
//Thread attributes are more than just flags;
//they also contain a number of sizes.
//This needs to be a complete struct.
#endif
