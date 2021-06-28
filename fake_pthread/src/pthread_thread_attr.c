// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include "pthread_thread_attr.h"

int   fake_pthread_attr_init(pthread_attr_t *attrs) {
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    attrs->detach_state = PTHREAD_CREATE_JOINABLE;
    attrs->guard_size = getpagesize();
    attrs->inherit_sched=PTHREAD_INHERIT_SCHED;
    attrs->sparam = NULL;
    attrs->sched_policy = 0;
    attrs->scope = 0;
    attrs->stack_addr = NULL;
    attrs->stack_size = PTHREAD_STACK_MIN; 
    return 0;
}
int   fake_pthread_attr_destroy(pthread_attr_t *attrs) {
    return 0;
}

int   fake_pthread_attr_getdetachstate(const pthread_attr_t *attrs, int *cur_state) {
    if(attrs == NULL || cur_state == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    *cur_state = attrs->detach_state;
    return 0;
}

int   fake_pthread_attr_setdetachstate(pthread_attr_t *attrs, int new_state) {
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    if(new_state < 0 || new_state > 1) {
        errno = EINVAL;
        return EINVAL;
    }
    attrs->detach_state = new_state;
    return 0;
}

int   fake_pthread_attr_getguardsize(const pthread_attr_t *attrs, size_t *cur_size) {
    if(attrs == NULL || cur_size == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    *cur_size = attrs->guard_size;
    return 0;
}

int   fake_pthread_attr_setguardsize(pthread_attr_t *attrs, size_t new_size) {
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    attrs->guard_size = new_size;
    return 0;
}

int   fake_pthread_attr_getinheritsched(const pthread_attr_t *attrs, int *cur_inherit) {
    if(attrs == NULL || cur_inherit == 0) {
        errno = EINVAL;
        return EINVAL;
    }
    *cur_inherit = attrs->inherit_sched;
    return 0;
}

int   fake_pthread_attr_setinheritsched(pthread_attr_t *attrs, int new_inherit) {
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    if(new_inherit < 0 || new_inherit > 1) {
        errno = EINVAL;
        return EINVAL;
    }
    attrs->inherit_sched = new_inherit;
    return 0;
}

int   fake_pthread_attr_getschedparam(const pthread_attr_t *attrs,
          struct sched_param *cur_param) {
    if(attrs == NULL || cur_param == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    return 0;
}
int   fake_pthread_attr_setschedparam(pthread_attr_t *attrs,
        const struct sched_param *new_param) { 
    if(attrs == NULL || new_param == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    attrs->sparam = (struct sched_param*)new_param;
    return 0;
}

int   fake_pthread_attr_getschedpolicy(const pthread_attr_t *attrs, int *cur_policy) {
    if(attrs == NULL || cur_policy == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    *cur_policy = attrs->sched_policy;
    return 0;
}

int   fake_pthread_attr_setschedpolicy(pthread_attr_t *attrs, int new_policy) {
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    attrs->sched_policy = new_policy;
    return 0;
}

int   fake_pthread_attr_getscope(const pthread_attr_t *attrs, int *cur_scope) {
    if(attrs == NULL || cur_scope == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    *cur_scope = attrs->scope;
    return 0;
}

int   fake_pthread_attr_setscope(pthread_attr_t *attrs, int new_scope) {
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    attrs->scope = new_scope;
    return 0;
}
int   fake_pthread_attr_getstackaddr(const pthread_attr_t *attrs, void **cur_addr) {
    if(attrs == NULL || cur_addr == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    *cur_addr = attrs->stack_addr;
    return 0;
}

int   fake_pthread_attr_setstackaddr(pthread_attr_t *attrs, void *new_addr) {
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    attrs->stack_addr = new_addr;
    return 0;
}

int   fake_pthread_attr_getstacksize(const pthread_attr_t *attrs, size_t *cur_size) {
    if(attrs == NULL || cur_size == NULL)  {
        errno = EINVAL;
        return EINVAL;
    }
    *cur_size = attrs->stack_size;
    return 0;
}

int   fake_pthread_attr_setstacksize(pthread_attr_t *attrs, size_t new_size) {
    if(attrs == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    if(new_size < PTHREAD_STACK_MIN) {
        errno = EINVAL;
        return EINVAL;
    }
    attrs->stack_size = new_size;
    return 0;
}
