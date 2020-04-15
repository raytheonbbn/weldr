// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#ifndef __PTHREAD_LAUNCH_H__
#define __PTHREAD_LAUNCH_H__
#include "pthread_shared.h"
#include "pthread_thread.h"
#include <setjmp.h>

void pthread_launch(void *stack_addr, jmp_buf buf, void *(*start_routine)(void*), void *arg);

#endif
