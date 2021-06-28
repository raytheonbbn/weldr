// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#ifndef __PTHREAD_CONDATTR_H__
#define __PTHREAD_CONDATTR_H__
#include "pthread_shared.h"
#include <errno.h>
#include <stdlib.h>


typedef int pthread_condattr_t;

int fake_pthread_condattr_init(pthread_condattr_t *attrs);
int fake_pthread_condattr_destroy(pthread_condattr_t *attrs);

int fake_pthread_condattr_getpshared(const pthread_condattr_t *attrs, int *cur_pshared);
int fake_pthread_condattr_setpshared(pthread_condattr_t *attrs, int new_pshared);

#endif
