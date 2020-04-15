// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "pthread_thread.h"
#include "pthread_struct.h"
#include "pthread_control.h"
#include <stdio.h>

int fake_pthread_create(pthread_t *thread, pthread_attr_t *attrs,
        void *(*start_routine)(void*), void *arg) {
    if(thread == NULL) {
        errno = EINVAL;
        return EINVAL;
    }


    pthread_t self_tid = fake_pthread_self();
    struct pthread_struct *self = pthread_get_struct_by_tid(self_tid);
    self->state = PTHREAD_STATE_RUNNABLE;

    pthread_t tid = 0;
    struct pthread_struct *pthread = pthread_get_new_struct(&tid);
    if(pthread == NULL) {
        errno = EAGAIN;
        return EAGAIN; 
    }

    pthread->valid = 42;
    pthread->tid = tid;
    pthread->state = PTHREAD_STATE_ACTIVE;
    pthread->detached = 0;
    pthread->num_user_cleanup_routines = 0;
    pthread->num_system_cleanup_routines = 0;
    pthread->cancel_state = 0;
    pthread->cancel_type = 0;
    pthread->canceled = 0;
    fake_pthread_mutex_init(&(pthread->join_mutex), NULL);
    fake_pthread_cond_init(&(pthread->join_cond), NULL);
    pthread->join_val = NULL;

    pthread_set_current_thread(tid);

    //Allocate the stack.  This may cause problems.
    //TODO: The documentation for MAP_GROWSDOWN isn't easy to read.
    pthread->stack_addr = mmap(NULL, PTHREAD_STACK_MIN, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN | MAP_STACK, -1, 0) + PTHREAD_STACK_MIN - 0x10; 
    pthread->stack_size = PTHREAD_STACK_MIN;

    
    //printf("jmp_buf before launch: %x\n", ((void *)self->hang_point));
    //printf("Mangled PC value before launch: %x\n", *((void**)(((void *)self->hang_point) + 0x38)));

    pthread_launch(pthread->stack_addr, self->hang_point, start_routine, arg);
    
    *thread = tid;

    return 0;
}

pthread_t fake_pthread_self() {
    return pthread_get_current_thread();
}

int fake_pthread_equal(pthread_t a, pthread_t b) {
    return a == b;
}

int fake_pthread_detach(pthread_t thread) {
    struct pthread_struct *pthread = pthread_get_struct_by_tid(thread);
    if(pthread == NULL) {
        errno = EINVAL;
        return EINVAL;
    }

    if(pthread->detached) {
        errno = EINVAL;
        return EINVAL;
    }

    if(pthread->state == PTHREAD_STATE_ZOMBIE) {
        //TODO: Cleanup the thread's memory.
        return 0;
    }

    pthread->detached=1;
    //TODO: Signal the join lock.
    return 0;
}

int fake_pthread_join(pthread_t thread, void **val) {
    //You can't join yourself.
    pthread_t self = fake_pthread_self();
    if(thread == self) {
        errno = EDEADLK;
        return EDEADLK;
    }
    //printf("Thread %lu is joining thread %lu\n", self, thread);

    struct pthread_struct *pthread = pthread_get_struct_by_tid(thread);
    if(pthread == NULL) {
        fprintf(stderr, "Thread %lu is missing; can't join.\n", thread);
        errno = EINVAL;
        return EINVAL;
    }

    //Do the easy case; check if the thread is detached.
    if(pthread->detached) {
        fprintf(stderr, "Thread %lu is detached; can't join.\n", thread);
        errno = EINVAL;
        return EINVAL;
    }

    //Check if the thread was joined
    //Because these aren't true threads, this section isn't
    //interleavable; only one thread will make this check at a time.
    //As such, only one thread will actually use the waiting loop below.
    if(pthread->join_registered) {
        fprintf(stderr, "Thread %lu is already being joined; can't join.\n", thread);
        errno = EINVAL;
        return EINVAL;
    } else {
        pthread->join_registered = 1;
    }

    int ret = 0;
    //Lock off the thread's joining state.
    if((ret = fake_pthread_mutex_lock(&(pthread->join_mutex)))) {
        return ret;
    }

    do {
        //If the thread was detached, ditch.
        if(pthread->detached) {
            fprintf(stderr, "Thread %lu was detached while we were locking; can't join.\n", thread);
            errno = EINVAL;
            return EINVAL;
        }

        //If the thread has exited cleanly, record the value
        //and shut it down.
        if(pthread->state == PTHREAD_STATE_ZOMBIE) {
            //printf("Thread %lu is a zombie (%u).  Cleaning house.\n", thread, pthread->state);
            if(val != NULL) {
                *val = pthread->join_val;
            }
            pthread_shutdown(pthread->tid);
            return 0;
        }

        //If neither have happened yet, sleep.
        if((ret = fake_pthread_cond_wait(&(pthread->join_cond), &(pthread->join_mutex)))) {
            fprintf(stderr, "Thread %lu failed to wait; can't join.\n", thread);
            return ret;
        }

    } while(1);    
}

void fake_pthread_exit(void *out) {
    pthread_t tid = fake_pthread_self();
    //This should never be null.
    struct pthread_struct *thread = pthread_get_struct_by_tid(tid);
    if(tid != thread->tid) {
        fprintf(stderr, "Tid missmatch during exit: self = %lu but struct = %lu\n", tid, thread->tid);
        exit(1);
    }
    
    //TODO: Call all of the cleanup functions.

    if(thread->detached) {
        //If detached, nuke from orbit.
        pthread_shutdown(thread->tid);
    } else {
        printf("Thread %lu is exiting.  Prepping join.\n", thread->tid);
        //Set the thread control block as joinable.
        thread->join_val = out;
        //Signal any waiting joiner to wake up.
        //printf("Thread %lu is signaling joiners.\n", thread->tid);
        fake_pthread_cond_signal(&(thread->join_cond));
        thread->state = PTHREAD_STATE_ZOMBIE;
        //Jump away from this thread.
        pthread_schedule();
    }
}


int   fake_pthread_getconcurrency(void) {
    return 0;
}

int   fake_pthread_setconcurrency(int new_concurrency) {
    return 0;
}

int   fake_pthread_getschedparam(pthread_t thread, int *cur_policy, struct sched_param *cur_param) {
    errno = ENOSYS;
    return ENOSYS;
}

int   fake_pthread_setschedparam(pthread_t thread, int new_policy,
          const struct sched_param *new_param) {
    errno = ENOSYS;
    return ENOSYS;
}

