// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "pthread_rwlockattr.h"


int   fake_pthread_rwlockattr_init(pthread_rwlockattr_t *attrs) {
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    *attrs = 0;
    return 0;

}
int   fake_pthread_rwlockattr_destroy(pthread_rwlockattr_t *attrs) {
    return 0;
}

int   fake_pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attrs,
          int *curr_pshared) {
    if(attrs == NULL || curr_pshared == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    *curr_pshared = *attrs;
    return 0;
}
int   fake_pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attrs, int new_pshared) {
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    if(new_pshared < 0 || new_pshared > 1) {
        errno = EINVAL;
        return EINVAL;
    }
    *attrs = new_pshared;
    return 0;
}
