// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "pthread_cancel.h"

int   fake_pthread_setcancelstate(int new_state, int *old_state) {
    if(new_state < 0 || new_state > 1) {
        errno = EINVAL;
        return EINVAL;
    }
    if(old_state == NULL) {
        errno = EINVAL;
        return EINVAL;
    }

    pthread_t tid = fake_pthread_self();
    struct pthread_struct *self = pthread_get_struct_by_tid(tid);
    *old_state = self->cancel_state;
    self->cancel_state = new_state; 

    return 0;
}

int   fake_pthread_setcanceltype(int new_type, int *old_type) {
    if(new_type < 0 || new_type > 1) {
        errno = EINVAL;
        return EINVAL;
    }
    if(old_type == NULL) {
        errno = EINVAL;
        return EINVAL;
    }

    pthread_t tid = fake_pthread_self();
    struct pthread_struct *self = pthread_get_struct_by_tid(tid);
    *old_type = self->cancel_type;
    self->cancel_type = new_type; 

    return 0;
}

void  fake_pthread_testcancel(void) {
    pthread_t tid = fake_pthread_self();
    struct pthread_struct *self = pthread_get_struct_by_tid(tid); 

    if(self->canceled) {
        //TODO: What argument gets passed here?
        fake_pthread_exit(0);
    }
}

int fake_pthread_cancel(pthread_t thread) {
    struct pthread_struct *self = pthread_get_struct_by_tid(thread);
    if(self == 0) {
        errno = ESRCH;
        return ESRCH;
    }

    if(self->cancel_state == PTHREAD_CANCEL_DISABLE) {
        return 0;
    }

    if(self->cancel_type == PTHREAD_CANCEL_ASYNCHRONOUS) {
        //TODO: Nuke the thread from orbit.
        //For now, report an error.
        errno = ENOSYS;
        return ENOSYS;
    } else {
        self->canceled = 1;
    }
}

void  fake_pthread_cleanup_push(void (*routine)(void *), void *arg) {
    pthread_t tid = fake_pthread_self();
    fake_pthread_cleanup_push_generic(tid, routine, arg, 0);
}

void fake_pthread_cleanup_pop(int execute) {
    pthread_t tid = fake_pthread_self();
    fake_pthread_cleanup_pop_generic(tid, execute, 0);
}
