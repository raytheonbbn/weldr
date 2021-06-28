// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include "pthread_once.h"

int fake_pthread_once(pthread_once_t *once_control, void(*init_routine)()) {
    if(!(*once_control)) {
        *once_control = 1;
        init_routine();
    }
    return 0;
}
