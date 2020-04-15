// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include <stdio.h>
#include "pthread_impl.h"
#include "semaphore_impl.h"

int main(int argc, char *argv[]) {
    printf("Size of pthread_t: %lu\n", sizeof(pthread_t));
    printf("Size of pthread_cond_t: %lu\n", sizeof(pthread_cond_t));
    printf("Size of pthread_mutex_t: %lu\n", sizeof(pthread_mutex_t));
    printf("Size of pthread_rwlock_t: %lu\n", sizeof(pthread_rwlock_t));
    printf("Size of pthread_key_t: %lu\n", sizeof(pthread_key_t));
    printf("Size of pthread_once_t: %lu\n", sizeof(pthread_once_t));
    printf("Size of sem_t: %lu\n", sizeof(sem_t));

    printf("Size of pthread_attr_t: %lu\n", sizeof(pthread_attr_t));
    printf("Size of pthread_condattr_t: %lu\n", sizeof(pthread_condattr_t));
    printf("Size of pthread_mutexattr_t: %lu\n", sizeof(pthread_mutexattr_t));
    printf("Size of pthread_rwlockattr_t: %lu\n", sizeof(pthread_rwlockattr_t));

    printf("Value of PTHREAD_CANCEL_ASYNCHRONOUS: %d\n", PTHREAD_CANCEL_ASYNCHRONOUS);
    printf("Value of PTHREAD_CANCEL_ENABLE: %d\n", PTHREAD_CANCEL_ENABLE);
    printf("Value of PTHREAD_CANCEL_DEFERRED: %d\n", PTHREAD_CANCEL_DEFERRED);
    printf("Value of PTHREAD_CANCELED: %d\n", PTHREAD_CANCELED);
    printf("Value of PTHREAD_CREATE_DETACHED: %d\n", PTHREAD_CREATE_DETACHED);
    printf("Value of PTHREAD_CREATE_JOINABLE: %d\n", PTHREAD_CREATE_JOINABLE);
    printf("Value of PTHREAD_EXPLICIT_SCHED: %d\n", PTHREAD_EXPLICIT_SCHED);
    printf("Value of PTHREAD_INHERIT_SHED: %d\n", PTHREAD_INHERIT_SCHED);
    printf("Value of PTHREAD_MUTEX_DEFAULT: %d\n", PTHREAD_MUTEX_DEFAULT);
    printf("Value of PTHREAD_MUTEX_NORMAL: %d\n", PTHREAD_MUTEX_NORMAL);
    printf("Value of PTHREAD_MUTEX_ERRORCHECK: %d\n", PTHREAD_MUTEX_ERRORCHECK);
    printf("Value of PTHREAD_MUTEX_RECURSIVE: %d\n", PTHREAD_MUTEX_RECURSIVE);
    printf("Value of PTHREAD_PRIO_INHERIT: %d\n", PTHREAD_PRIO_INHERIT);
    printf("Value of PTHREAD_PRIO_NONE: %d\n", PTHREAD_PRIO_NONE);
    printf("Value of PTHREAD_PRIO_PROTECT: %d\n", PTHREAD_PRIO_PROTECT);
    printf("Value of PTHREAD_PROCESS_SHARED: %d\n", PTHREAD_PROCESS_SHARED);
    printf("Value of PTHREAD_PROCESS_PRIVATE: %d\n", PTHREAD_PROCESS_PRIVATE);
    printf("Value of PTHREAD_SCOPE_PROCESS: %d\n", PTHREAD_SCOPE_PROCESS);
    printf("Value of PTHREAD_SCOPE_SYSTEM: %d\n", PTHREAD_SCOPE_SYSTEM);
}
