// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "pthread_mutexattr.h"

int   fake_pthread_mutexattr_init(pthread_mutexattr_t *attrs) {
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL; 
    }
    *attrs = PTHREAD_MUTEXATTR_DEFAULT;
    return 0;
}

int   fake_pthread_mutexattr_destroy(pthread_mutexattr_t *atts) {
    return 0;
}

int   fake_pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attrs, int *curr_protocol)  {
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL; 
    }
    *curr_protocol = *attrs & 3;
    return 0;
}

int   fake_pthread_mutexattr_setprotocol(pthread_mutexattr_t *attrs, int new_protocol) {
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    if(new_protocol < 0 || new_protocol > 2) {
        errno = ENOTSUP;
        return ENOTSUP; 
    }
#ifndef _PTHREAD_PRIO_INHERIT
    if(new_protocol == PTHREAD_PRIO_INHERIT) {
        errno = ENOSYS;
        return ENOSYS;
    }
#endif
#ifndef _PTHREAD_PRIO_PROTECT
    if(new_protocol == PTHREAD_PRIO_PROTECT) {
        errno = ENOSYS;
        return ENOSYS;
    }
#endif
    *attrs &= ~3;
    *attrs |= new_protocol;
    return 0;
}

int   fake_pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attrs, int *curr_ceiling) {
#ifndef _PTHREAD_PRIO_PROTECT
        errno = ENOSYS;
        return ENOSYS;
#else
    if(attrs == NULL || curr_ceiling == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    *curr_ceiling = *attrs >> 16;
    return 0;
#endif
}

int   fake_pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attrs, int new_ceiling) {
#ifndef _PTHREAD_PRIO_PROTECT
    errno = ENOSYS;
    return ENOSYS;
#else
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    if(new_ceiling > 0xFFFF) {
        return EINVAL;
    }
    *attrs &= 0xFFFF;
    *attrs |= new_ceiling << 16;
    return 0;
#endif
}

int   fake_pthread_mutexattr_getpshared(const pthread_mutexattr_t *attrs, int *curr_pshared) {
    if(attrs == NULL || curr_pshared == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    *curr_pshared = (*attrs >> 2) & 1;
    return 0;
}

int   fake_pthread_mutexattr_setpshared(pthread_mutexattr_t *attrs, int new_pshared) {
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    if(new_pshared > 1 || new_pshared < 0) {
        errno = EINVAL;
        return EINVAL;
    }
    *attrs &= ~(1 << 2);
    *attrs |= (new_pshared << 2);
    return 0;
}
int   fake_pthread_mutexattr_gettype(const pthread_mutexattr_t *attrs, int *curr_type) {
    if(attrs == NULL || curr_type == NULL) {
        errno = EINVAL;
        return EINVAL;
    }

    *curr_type = (*attrs >> 3) & 3;
    return 0;
}
int   fake_pthread_mutexattr_settype(pthread_mutexattr_t *attrs, int new_type)
{
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    if(new_type < 0 || new_type > 2) {
        errno = EINVAL;
        return EINVAL;
    }
    *attrs &= ~(3 << 3);
    *attrs |= (new_type << 3);
    
    return 0;
}
