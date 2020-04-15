// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#ifndef __PTHREAD_THREAD_H__
#define __PTHREAD_THREAD_H__
#include "pthread_shared.h"
#include "pthread_thread_attr.h"
#include "pthread_launch.h"
#include "sched.h"

#include <stdlib.h>

//Extensions of the pthread library.
void init_pthread_globals();

//Get a pointer to the pthread struct by its tid.
struct pthread_struct *pthread_get_struct_by_tid(pthread_t thread);

struct pthread_struct *pthread_get_new_struct(pthread_t *tid);

//Wrapper for launching a thread; calls exit when the startup routine leaves.
void pthread_launch_helper(void* addr, jmp_buf buf, void *(*startup_routine)(void*), void *arg); 

//Push/pop/remove cleanup args.
//isSystem specifies which array will get poked.
void fake_pthread_cleanup_push_generic(pthread_t tid, void  (*routine)(void *), const void *arg, int isSystem);
void fake_pthread_cleanup_pop_generic(pthread_t tid, int exec, int isSystem);
int pthread_cleanup_remove_generic(pthread_t tid, void (*remove)(void *), int isSystem);
int pthread_cleanup_swaparg_generic(pthread_t tid, void (*swap)(void *), const void *new_arg, int isSystem);

//Launch a new thread.
//This will create a new thread running the function stored in
//start_routine, which will get called with arg as its argument.
int   fake_pthread_create(pthread_t *thread, pthread_attr_t *attrs, 
        void *(*start_routine)(void*), void *arg);

//Get the current thread's thread handle.
pthread_t fake_pthread_self(void);

//Check if two thread handles reference the same thread.
int   fake_pthread_equal(pthread_t a, pthread_t b);

//Detach the specified thread.
//A detached thread will clean up its resources
//immediately on exiting; you can't and don't need to call join on it.
int   fake_pthread_detach(pthread_t thread);

//Block until the specified thread exits.
//If the pointer in out is not null, the value passed
//into the target thread's exit will be stored in it.
//Once the thread is joined, its resources will be reclaimed.
int   fake_pthread_join(pthread_t thread, void **out);

//Exit the thread.
//This will call a bunch of cleanup handles before terminitating.
void  fake_pthread_exit(void *out);

//Get/set the thread's concurrency level.
//This is used to support user-threat-to-kernel-thread
//multiplexing.  Since we have no kernel threads,
//we don't support multiplexing, and thus this does nothing.
int   fake_pthread_getconcurrency(void);
int   fake_pthread_setconcurrency(int new_concurrency);

//Get/set the scheduler parameters.
int   fake_pthread_getschedparam(pthread_t thread, int *cur_policy, struct sched_param *cur_param);
int   fake_pthread_setschedparam(pthread_t thread, int new_policy,
          const struct sched_param *new_param);


//Design
//
//pthread_t:
//
//This is a conundrum.  Pthread objects are normally
//just IDs that hook into a hidden data structure.
//Because the design supports passing pthreads directly,
//we need to use something similar; the library
//needs to maintain its own list of threads.
//
//Thread execution:
//
//If a thread is active, it literally is the main thread of the application.
//The trick is to simulate a thread blocking and transferring control to another runnable thread.
//This is actually also easy; C already gives us the ability to do this using set_jump/long_jump.
//
//The real trick is thread creation, when we need to allocate a new thread stack and poke
//the processor's registers to use it.

#endif
