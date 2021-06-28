// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#ifndef __FAKE_SETJMP_H__
#define __FAKE_SETJMP_H__

#include <setjmp.h>

int fake_setjmp(jmp_buf buf);

int fake_longjmp(jmp_buf buf, int out);


#endif
