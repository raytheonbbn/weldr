// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include "pthread_impl.h"
#include <stdio.h>

INIT_WRAPPER(fake_pthread) {
    init_pthread_globals();
    init_pthread_key();

    //Allocate a pthread struct for the main thread.
    pthread_t tid = 0;
    struct pthread_struct *thread = pthread_get_new_struct(&tid);
    if(thread == NULL) {
        fprintf(stderr, "Main thread struct was null\n");
        return 1; 
    }
    if(tid != 0) {
        fprintf(stderr, "Main thread allocated as %lu, not 0\n", tid);
        return 1;
    } else {
        //printf("Main thread allocated as 0, according to library...\n");
    }

    pthread_set_current_thread(0);

    thread->valid = 42;
    thread->tid = tid;
    thread->state = PTHREAD_STATE_ACTIVE;
    thread->detached = 0;
    thread->num_user_cleanup_routines = 0;
    thread->num_system_cleanup_routines = 0;
    thread->cancel_state = 0;
    thread->cancel_type = 0;
    thread->canceled = 0;
    fake_pthread_mutex_init(&(thread->join_mutex), NULL);
    fake_pthread_cond_init(&(thread->join_cond), NULL);
    thread->join_val = NULL;


    if(fake_pthread_self() != 0) {
        fprintf(stderr, "Didn't detect thread 0 as the active thread.\n");
        return 1;
    }

    return 0;
}



int __wrap_pthread_attr_destroy(pthread_attr_t *attrs) {
    return fake_pthread_attr_destroy(attrs);
}

int __wrap_pthread_attr_getdetachstate(const pthread_attr_t *attrs, int *cur_state) {
    return fake_pthread_attr_getdetachstate(attrs, cur_state);
}

int __wrap_pthread_attr_getguardsize(const pthread_attr_t *attrs, size_t *cur_size) {
    return fake_pthread_attr_getguardsize(attrs, cur_size);
}

int __wrap_pthread_attr_getinheritsched(const pthread_attr_t *attrs, int *cur_inherit) {
    return fake_pthread_attr_getinheritsched(attrs, cur_inherit);
}

int __wrap_pthread_attr_getschedparam(const pthread_attr_t * attrs,
          struct sched_param *cur_param) {
    return fake_pthread_attr_getschedparam(attrs, cur_param);
}

int __wrap_pthread_attr_getschedpolicy(const pthread_attr_t *attrs, int *cur_policy) {
    return fake_pthread_attr_getschedpolicy(attrs, cur_policy);
}

int __wrap_pthread_attr_getscope(const pthread_attr_t *attrs, int *cur_scope) {
    return fake_pthread_attr_getscope(attrs, cur_scope);
}

int __wrap_pthread_attr_getstackaddr(const pthread_attr_t *attrs, void **cur_addr) {
    return fake_pthread_attr_getstackaddr(attrs, cur_addr);
}

int __wrap_pthread_attr_getstacksize(const pthread_attr_t *attrs, size_t *cur_size) {
    return fake_pthread_attr_getstacksize(attrs, cur_size);
}

int __wrap_pthread_attr_init(pthread_attr_t *attrs) {
    return fake_pthread_attr_init(attrs);
}

int __wrap_pthread_attr_setdetachstate(pthread_attr_t *attrs, int new_state) {
    return fake_pthread_attr_setdetachstate(attrs, new_state);
}

int __wrap_pthread_attr_setguardsize(pthread_attr_t *attrs, size_t new_size) {
    return fake_pthread_attr_setguardsize(attrs, new_size);
}

int __wrap_pthread_attr_setinheritsched(pthread_attr_t *attrs, int new_inherit) {
    return fake_pthread_attr_setinheritsched(attrs, new_inherit);
}

int __wrap_pthread_attr_setschedparam(pthread_attr_t *attrs,
          const struct sched_param *new_param) {
    return fake_pthread_attr_setschedparam(attrs, new_param);
}

int __wrap_pthread_attr_setschedpolicy(pthread_attr_t *attrs, int new_policy) {
    return fake_pthread_attr_setschedpolicy(attrs, new_policy);
}

int __wrap_pthread_attr_setscope(pthread_attr_t *attrs, int new_scope) {
    return fake_pthread_attr_setscope(attrs, new_scope);
}

int __wrap_pthread_attr_setstackaddr(pthread_attr_t *attrs, void *new_addr) {
    return fake_pthread_attr_setstackaddr(attrs, new_addr);
}

int __wrap_pthread_attr_setstacksize(pthread_attr_t *attrs, size_t new_size) {
    return fake_pthread_attr_setstacksize(attrs, new_size);
}

int __wrap_pthread_cancel(pthread_t tid) {
    return fake_pthread_cancel(tid);
}

void __wrap_pthread_cleanup_push(void (*routine)(void*), void *arg) {
    fake_pthread_cleanup_push(routine, arg);
}

void __wrap_pthread_cleanup_pop(int exec) {
    fake_pthread_cleanup_pop(exec);
}

int __wrap_pthread_cond_broadcast(pthread_cond_t *cond) {
    return fake_pthread_cond_broadcast(cond);
}

int __wrap_pthread_cond_destroy(pthread_cond_t *cond) {
    return fake_pthread_cond_destroy(cond);
}

int __wrap_pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attrs) {
    return fake_pthread_cond_init(cond, attrs);
}

int __wrap_pthread_cond_signal(pthread_cond_t *cond) {
    return fake_pthread_cond_signal(cond);
}

int __wrap_pthread_cond_timedwait(pthread_cond_t *cond, 
        pthread_mutex_t *mutex, const struct timespec *timeout) {
    return fake_pthread_cond_timedwait(cond, mutex, timeout);
}

int __wrap_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
    return fake_pthread_cond_wait(cond, mutex);
}

int __wrap_pthread_condattr_destroy(pthread_condattr_t *attrs) {
    return fake_pthread_condattr_destroy(attrs);
}

int __wrap_pthread_condattr_getpshared(const pthread_condattr_t *attrs, int *cur_shared) {
    return fake_pthread_condattr_getpshared(attrs, cur_shared);
}

int __wrap_pthread_condattr_init(pthread_condattr_t *attrs) {
    return fake_pthread_condattr_init(attrs);
}

int __wrap_pthread_condattr_setpshared(pthread_condattr_t *attrs, int new_shared) {
    return fake_pthread_condattr_setpshared(attrs, new_shared);
}

int __wrap_pthread_create(pthread_t *tid, const pthread_attr_t *attrs,
          void *(*startup_routine)(void *), void *arg) {
    return fake_pthread_create(tid, (pthread_attr_t*)attrs, startup_routine, arg);
}

int __wrap_pthread_detach(pthread_t tid) {
    return fake_pthread_detach(tid);
}

int __wrap_pthread_equal(pthread_t a, pthread_t b) {
    return fake_pthread_equal(a, b);
}

void __wrap_pthread_exit(void *arg) {
    fake_pthread_exit(arg);
}
int __wrap_pthread_getconcurrency(void) {
    return fake_pthread_getconcurrency();
}

int __wrap_pthread_getschedparam(pthread_t tid, int *policy, struct sched_param *param) {
    return fake_pthread_getschedparam(tid, policy, param);
}
void *__wrap_pthread_getspecific(pthread_key_t key) {
    return fake_pthread_getspecific(key);
}
int __wrap_pthread_join(pthread_t tid, void **arg) {
    return fake_pthread_join(tid, arg);
}

int __wrap_pthread_key_create(pthread_key_t *key, void (*destructor)(void *)) {
    return fake_pthread_key_create(key, destructor);
}

int __wrap_pthread_key_delete(pthread_key_t key) {
    return fake_pthread_key_delete(key);
}

int __wrap_pthread_mutex_destroy(pthread_mutex_t *mutex) {
    return fake_pthread_mutex_destroy(mutex);
}

int __wrap_pthread_mutex_getprioceiling(const pthread_mutex_t *mutex, int *cur_ceiling) {
    return fake_pthread_mutex_getprioceiling(mutex, cur_ceiling);
}

int __wrap_pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attrs) {
    return fake_pthread_mutex_init(mutex, attrs);
}

int __wrap_pthread_mutex_lock(pthread_mutex_t *mutex) {
    return fake_pthread_mutex_lock(mutex);
}

int __wrap_pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int new_ceiling, int *old_ceiling) {
    return fake_pthread_mutex_setprioceiling(mutex, new_ceiling, old_ceiling);
}

int __wrap_pthread_mutex_trylock(pthread_mutex_t *mutex) {
    return fake_pthread_mutex_trylock(mutex);
}

int __wrap_pthread_mutex_unlock(pthread_mutex_t *mutex) {
    return fake_pthread_mutex_unlock(mutex);
}

int __wrap_pthread_mutexattr_destroy(pthread_mutexattr_t *attrs) {
    return fake_pthread_mutexattr_destroy(attrs);
}

int __wrap_pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attrs,
          int *cur_ceiling) {
    return fake_pthread_mutexattr_getprioceiling(attrs, cur_ceiling);
}

int __wrap_pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attrs, int *cur_protocol) {
    return fake_pthread_mutexattr_getprotocol(attrs, cur_protocol);
}

int __wrap_pthread_mutexattr_getpshared(const pthread_mutexattr_t *attrs, int *cur_pshared) {
    return fake_pthread_mutexattr_getpshared(attrs, cur_pshared);
}

int __wrap_pthread_mutexattr_gettype(const pthread_mutexattr_t *attrs, int *cur_type) {
    return fake_pthread_mutexattr_gettype(attrs, cur_type);
}

int __wrap_pthread_mutexattr_init(pthread_mutexattr_t *attrs) {
    return fake_pthread_mutexattr_init(attrs);
}

int __wrap_pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attrs, int new_ceiling) {
    return fake_pthread_mutexattr_setprioceiling(attrs, new_ceiling);
}

int __wrap_pthread_mutexattr_setprotocol(pthread_mutexattr_t *attrs, int new_protocol) {
    return fake_pthread_mutexattr_setprotocol(attrs, new_protocol);
}

int __wrap_pthread_mutexattr_setpshared(pthread_mutexattr_t *attrs, int new_pshared) {
    return fake_pthread_mutexattr_setpshared(attrs, new_pshared);
}

int __wrap_pthread_mutexattr_settype(pthread_mutexattr_t *attrs, int new_type) {
    return fake_pthread_mutexattr_settype(attrs, new_type);
}

int __wrap_pthread_once(pthread_once_t *once, void (*routine)(void)) {
    return fake_pthread_once(once, routine);
}

int __wrap_pthread_rwlock_destroy(pthread_rwlock_t *rwlock) {
    return fake_pthread_rwlock_destroy(rwlock);
}

int __wrap_pthread_rwlock_init(pthread_rwlock_t *rwlock,
          const pthread_rwlockattr_t *attrs) {
    return fake_pthread_rwlock_init(rwlock, attrs);
}

int __wrap_pthread_rwlock_rdlock(pthread_rwlock_t *rwlock) {
    return fake_pthread_rwlock_rdlock(rwlock);
}

int __wrap_pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock) {
    return fake_pthread_rwlock_tryrdlock(rwlock);
}

int __wrap_pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock) {
    return fake_pthread_rwlock_trywrlock(rwlock);
}

int __wrap_pthread_rwlock_unlock(pthread_rwlock_t *rwlock) {
    return fake_pthread_rwlock_unlock(rwlock);
}

int __wrap_pthread_rwlock_wrlock(pthread_rwlock_t *rwlock) {
    return fake_pthread_rwlock_wrlock(rwlock);
}

int __wrap_pthread_rwlockattr_destroy(pthread_rwlockattr_t *attrs) {
    return fake_pthread_rwlockattr_destroy(attrs);
}

int __wrap_pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attrs,
          int *cur_pshared) {
    return fake_pthread_rwlockattr_getpshared(attrs, cur_pshared);
}

int __wrap_pthread_rwlockattr_init(pthread_rwlockattr_t *attrs) {
    return fake_pthread_rwlockattr_init(attrs);
}

int __wrap_pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attrs, int new_pshared) {
    return fake_pthread_rwlockattr_setpshared(attrs, new_pshared);
}
pthread_t __wrap_pthread_self(void) {
    return fake_pthread_self();
}

int __wrap_pthread_sigmask(int how, const sigset_t *set, sigset_t *oldest) {
    // Welded binaries do not preserve signal-based behaviors.
    return 0;
}

int __wrap_pthread_setcancelstate(int new_state, int *old_state) {
    return fake_pthread_setcancelstate(new_state, old_state);
}

int __wrap_pthread_setcanceltype(int new_type, int *old_type) {
    return fake_pthread_setcanceltype(new_type, old_type);
}

int __wrap_pthread_setconcurrency(int new_concurrency) {
    return fake_pthread_setconcurrency(new_concurrency);
}

int __wrap_pthread_setschedparam(pthread_t tid, int policy,
          const struct sched_param *param) {
    return fake_pthread_setschedparam(tid, policy, param);
}

int __wrap_pthread_setspecific(pthread_key_t key, const void *val) {
    return fake_pthread_setspecific(key, val);
}

void __wrap_pthread_testcancel(void) {
    fake_pthread_testcancel();
}
