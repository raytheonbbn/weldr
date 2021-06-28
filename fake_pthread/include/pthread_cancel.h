// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#ifndef __PTHREAD_CANCEL_H__
#define __PTHREAD_CANCEL_H__
#include "pthread_shared.h"
#include "pthread_control.h"
#include "pthread_struct.h"

#include <errno.h>

//No idea where this gets used
#define PTHREAD_CANCELED -1

//Constants for cancel states. 
#define PTHREAD_CANCEL_ENABLE 0
#define PTHREAD_CANCEL_DISABLE 1

//Constants for cancel types.

//The thread will get cancelled the next
//time it or its dependencies call fake_pthread_testcancel.
#define PTHREAD_CANCEL_DEFERRED 0

//The thread will get cancelled ASAP.
//This is basically useless; the thread can't
//interact with the system or even the parent process
//with any guarantee of safety.
#define PTHREAD_CANCEL_ASYNCHRONOUS 1

//Configure the cancel parameters of the current thread.
//The default state is PTHREAD_CANCEL_ENABLE,
//and the default type is PTHREAD_CANCEL_DEFERRED
int   fake_pthread_setcancelstate(int new_state, int *old_state);
int   fake_pthread_setcanceltype(int new_type, int *old_type);

//Set a cancel point in the calling thread.
//This is roughly equivalent to checking the interrupted
//state of a Java thread; if the thread has canceling enabled,
//and cancel is set to deferred, this function will
//run the thread's cleanup routines and exit the thread.
//
//The idea of this function is to declare points where
//it is known to be safe to cancel the thread.
//In addition to letting the user define these points,
//a large number of libc functions have built-in cancel points.
void  fake_pthread_testcancel(void);

//Signal that a thread should cancel itself.
int   fake_pthread_cancel(pthread_t thread);

//Add a cleanup routine to the thread.
//Cleanup routines get called when a thread cancels,
//when it exits, or the thread calls cleanup_pop with a non-zero argument.
void  fake_pthread_cleanup_push(void (*routine)(void*), void *arg);

//Remove the last-registered cleanup routine.
//The routine will get executed if it's 
void  fake_pthread_cleanup_pop(int execute);

//Design
//
//Deferred cancellation is straightforward;
//if fake_pthread_testcancel gets called when the thread is cancelled, it calls exit.
//Async cancellation is simpler, but riskier.  Nuke the thread from orbit.
#endif
