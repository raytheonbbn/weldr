// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#ifndef __PTHREAD_ONCE_H__
#define __PTHREAD_ONCE_H__
#include "pthread_shared.h"

//Definitions for fake_pthread_once

//Coordinator type for fake_pthread_once.
typedef int pthread_once_t;

//Constant initializer for fake_pthread_once_t;
//The _only_ way to initialize fake_pthread_once_t
//is to execute "pthread_once_t once = PTHREAD_ONCE_INIT";
#define PTHREAD_ONCE_INIT 0;

//Call a function exactly once across multiple threads.
//
//pthread_once attempts to invoke a function (via the init_routine)
//pointer.  Behavior depends on the value stored in once_control;
//If this is the first thread to call fake_pthread_once with a given
//once_control object, init_routine is executed normally,
//after which the call to fake_pthread_once returns 0.
//If any other thread tries afterwards to execute fake_pthread_once with the same
//once_control object, fake_pthread_once returns immediately with a value of -1.
int fake_pthread_once(pthread_once_t *once_control, void(*init_routine)());

//Design:
//
//For our implementation, this doesn't need to be complicated.
//Since all control flow is serialized, fake_pthread_once_t
//can be as simple as an int.  If no one's called fake_pthread_once
//on that once_control, the int will be 0.  fake_pthread_once will
//set the int to 1, notifying all future calls that they should
//turn back.

#endif
