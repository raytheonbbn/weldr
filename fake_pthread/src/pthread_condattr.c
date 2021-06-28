// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include "pthread_condattr.h"

int fake_pthread_condattr_init(pthread_condattr_t *attrs) {
    //printf("\n >>> condattr_init\n");
    *attrs = 0; // This ensures attr is ALWAYS PRIVATE aka 0
    return 0;
}
int fake_pthread_condattr_destroy(pthread_condattr_t *attrs) {
    //printf("\n >>> condattr_destroy\n\n");
    return 0;
}

int   fake_pthread_condattr_getpshared(const pthread_condattr_t *attrs, int *cur_pshared) {
    //printf("\n >>> condattr_getshared\n");
    //printf("\n >>> cur_pshared = %d\n", *cur_pshared); // Should be 0
    if(cur_pshared == NULL) {
        //TODO: In what other cases is cur_pshared invalid?
        errno = EINVAL;
        return EINVAL;
    }

    *cur_pshared = *attrs;
    return 0;
}

int   fake_pthread_condattr_setpshared(pthread_condattr_t *attrs, int new_pshared) {
    //printf("\n >>> condattr_setshared\n");
    //printf("\n >>> new_pshared = %d \n", new_pshared);  
    if(new_pshared < 0 || new_pshared > 1) {
        errno = EINVAL;
        return EINVAL;
    }
    *attrs = new_pshared;
    return 0;
}
