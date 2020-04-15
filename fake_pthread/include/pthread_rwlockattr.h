// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#ifndef __PTHREAD_RWLOCKATTR_H__
#define __PTHREAD_RWLOCKATTR_H__
#include "pthread_shared.h"

#include <errno.h>
#include <stdlib.h>

//Definiton for rwlock attribute objects.

typedef long unsigned int pthread_rwlockattr_t;

#define PTHREAD_RWLOCKATTR_DEFAULT 0; 

//Initalize a rwlockattr
int   fake_pthread_rwlockattr_init(pthread_rwlockattr_t *attrs);
//Destroy a rwlockattr
int   fake_pthread_rwlockattr_destroy(pthread_rwlockattr_t *attrs);

//Get/set the current process sharing policy.
//
int   fake_pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attrs,
          int *curr_pshared);
int   fake_pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attrs, int new_pshared);

//Design
//
//This is another glorified batch of flags.
//In fact, it's exactly one flag.

#endif
