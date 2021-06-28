// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#ifndef __PTHREAD_CONTROL_H__
#define __PTHREAD_CONTROL_H__
#include "pthread_shared.h"

void pthread_set_current_thread(pthread_t tid);
pthread_t pthread_get_current_thread();

//Helper function for restarting a suspended thread.
int pthread_schedule();

//Block a thread.
//This means setting the thread's jumpbuf
//and picking a runnable thread out of the array
//to take up execution.
int pthread_block(pthread_t thread);

//Wake a sleeping thread.
//This means re-enabling the thread
//and using its jumpbuf to jump back into execution. 
int pthread_wake(pthread_t thread);

//Clear up the thread's resources.
int pthread_shutdown(pthread_t thread);

#endif
