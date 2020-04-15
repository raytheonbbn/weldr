// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#ifndef __PTHREAD_MUTEXATTR_H__
#define __PTHREAD_MUTEXATTR_H__
#include "pthread_shared.h"

#include <errno.h>
#include <stdlib.h>
//Define mutex attribute structures

//Typedef for pthread_mutexattr_t.
typedef int pthread_mutexattr_t;

//Default value of a mutexattr; just the int 0.
#define PTHREAD_MUTEXATTR_DEFAULT 0;

int   fake_pthread_mutexattr_init(pthread_mutexattr_t *attrs);
int   fake_pthread_mutexattr_destroy(pthread_mutexattr_t *atts);

//#define _PTHREAD_PRIO_INHERIT
//#define _PTHREAD_PRIO_PROTECT

#define PTHREAD_PRIO_NONE 0
#define PTHREAD_PRIO_INHERIT 1
#define PTHREAD_PRIO_PROTECT 2

//Get/set the mutex scheduler protocol.
//
//The protocol defines how a thread's priority
//and the mutex's priority ceiling interact with the scheduler.
//- If the protocol is set to PTHREAD_PRIO_NONE, priority checking is disabled.
//- If it's set to PTHREAD_PRIO_INHERIT, threads locking this mutex have 
//  their priorities boosted to match the mutex's priority ceiling until they unlock.
//- If it's set to PTHREAD_PRIO_PROTECT, threads with higher priorities than
//  this mutex's priority ceiling will not be allowed to aquire the mutex.
//- If a thread attempts a lock on a mutex set to PTHREAD_PRIO_INHERIT, and is blocked,
//  the current owner gets its priority temporarily increased to the priority of the
//  blocked thread (if the blocked thread is of higher priority than the owner is currently.)
//  This happens recursively; if the owner is itself blocked on a mutex with the INHERIT protocol,
//  the owner of _that_ mutex gets its priority bumped using the temporary priority.
//
//  INHERIT and PROTECT may not be implemented; if a program tries to use one that's not implemented,
//  mutex calls will return an error code.
int   fake_pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attrs, int *curr_protocol);
int   fake_pthread_mutexattr_setprotocol(pthread_mutexattr_t *attrs, int new_protocol);

//Get/set the priority ceiling.
//The impact of this value depends on the mutex protocol; see
//get/setprotocol.  The maximum ceiling depends on the scheduler implementation.
int   fake_pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attrs,
          int *curr_ceiling);
int   fake_pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attrs, int new_ceiling);

//Get/set the process-shared attribute of the mutex.
//- If set to PROCESS_SHARED, the mutex can be accessed by any thread that can reach
//  the memory that holds the mutex, even if the thread is from a different process.
//- If set to PROCESS_PRIVATE, only threads from the process that owns the memory
//  can use the mutex; other threads will get an error code.
//The default value of a mutex is PROCESS_PRIVATE.
int   fake_pthread_mutexattr_getpshared(const pthread_mutexattr_t *attrs, int *curr_pshared);
int   fake_pthread_mutexattr_setpshared(pthread_mutexattr_t *attrs, int new_pshared);


#define PTHREAD_MUTEX_NORMAL 0
#define PTHREAD_MUTEX_RECURSIVE 1
#define PTHREAD_MUTEX_ERRORCHECK 2
//Redefine default as errorcheck.  I like error handling.
#define PTHREAD_MUTEX_DEFAULT PTHREAD_MUTEX_NORMAL

//Get/set the mutex algorithm type.
//- If set to PTHREAD_MUTEX_NORMAL, the mutex doesn't attempt deadlock detection; 
//  trying to re-lock the same mutex will deadlock.
//  trying to unlock someone else's mutex is undefined behavior.
//- If set to PTHREAD_MUTEX_ERRORCHECK, the mutex refuses to allow self-deadlock;
//  trying to re-lock the same mutex will return an error.
//  trying to unlock someone else's mutex will return an error.
//- If set to PTHREAD_MUTEX_RECURSIVE, the mutex is fully reentrant.
//  trying to re-lock the same mutex will increment an internal counter;
//  the thread only fully releases the lock once it's unlocked it the 
//  same number of times as it locked.
//  trying to unlock someone else's mutex will return an error.
//- If set to PTHREAD_MUTEX_DEFAULT, behavior is implementation-specifc.
//  The library either maps this to another value, or bad behavior results in 'splosions.
//The default algorithm type is (surprise, surprise) PTHREAD_MUTEX_DEFAULT
int   fake_pthread_mutexattr_gettype(const pthread_mutexattr_t *, int *);
int   fake_pthread_mutexattr_settype(pthread_mutexattr_t *, int);

//Design:
//
//This is an exceptionally glorified batch of flags.
//General design:
//- Bits 0 and 1: protocol
//- Bit 2: pshared
//- Bits 3 and 4: type
//- Bits 5-15: Unused.
//- Bits 16-31: prioceiling (defined as short.)
//Parts of protocol are okay to implement,

#endif
