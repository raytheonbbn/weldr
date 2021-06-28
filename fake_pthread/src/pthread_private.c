// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include "pthread_struct.h"
#include "pthread_control.h"
#include <stdio.h>

unsigned int thread_mask;
pthread_t current_thread;
struct pthread_struct threads[PTHREAD_MAX_THREADS];

void init_pthread_globals() {
        thread_mask = 0;
}

void pthread_set_current_thread(pthread_t tid) {
    current_thread = tid;
}

pthread_t pthread_get_current_thread() {
    return current_thread;
}

struct pthread_struct *pthread_get_struct_by_tid(pthread_t thread) {
    if(!(thread_mask & 1 << (thread))) {
        return NULL;
    }

    return &(threads[thread]);
}

struct pthread_struct *pthread_get_new_struct(pthread_t *tid) {
    if(thread_mask == 0xFFFFFFFF) {
        fprintf(stderr, "FATAL: Out of available threads: %d\n", thread_mask);
        return NULL;
    }
    struct pthread_struct *out = NULL;

    for(int i = 0; i < PTHREAD_MAX_THREADS; i++) {
        if(!(thread_mask & (1 << i))) {
            fprintf(stderr, "tid %d is available\n", i);
            *tid = i;
            out = &(threads[i]);
            thread_mask |= (1 << i);
            break;
        }
    }
    return out;
    
}

void pthread_launch_helper(void *stack_addr, jmp_buf hang_point, void *(*start_routine)(void*), void *arg) {
    void *out = (*start_routine)(arg);
    printf("Exiting thread with status %p\n", out);
    fake_pthread_exit(out);
}

int pthread_schedule() {
    //Ensure we cycle all threads first.
    for(int i = 1; i <= PTHREAD_MAX_THREADS; i++) {
        int idx = (current_thread + i) % PTHREAD_MAX_THREADS;
        //Check if this thread is allocated.
        if(thread_mask & (1 << idx)) {
            struct pthread_struct *new_pthread = &(threads[idx]);
            //Check if this thread is unblocked.
            if(new_pthread->state == PTHREAD_STATE_RUNNABLE) {
                printf("\tRestarting thread %u\n", idx);
                pthread_set_current_thread(idx);
                new_pthread->state = PTHREAD_STATE_ACTIVE;
                longjmp(new_pthread->hang_point, 1); //NOTE: doesn't return is cross procedural goto, so no return is needed for this case
            } else {
                printf("\tThread %u exists, but isn't runnable: expected %d vs actual %d\n", idx, PTHREAD_STATE_RUNNABLE, new_pthread->state);
                continue;
            }
        }
    }
    //No threads were available to run.  WTH?
    fprintf(stderr, "\tNo threads available to run.  WTH??\n");
    exit(-1);
    return EDEADLK;
}

int pthread_block(pthread_t thread) {
    struct pthread_struct *pthread = pthread_get_struct_by_tid(thread);

    if(pthread == NULL) {
        errno = EINVAL;
        return EINVAL;
    }

    //printf("Blocking thread %u\n", thread);
    //Set the state to blocked, and mark this as the restart point.
    pthread->state = PTHREAD_STATE_BLOCKED;
    int res = setjmp(pthread->hang_point);

    //Check the setjmp result.  If not zero, we're being woken.  Don't sleep.
    if(res) {
        return 0;
    } else {
        return pthread_schedule();
    }
}

int pthread_wake(pthread_t thread) {
    struct pthread_struct *pthread = pthread_get_struct_by_tid(thread);
    if(pthread == NULL) {
        errno = EINVAL;
        return EINVAL;
    }

    pthread->state = PTHREAD_STATE_RUNNABLE;
    //printf("Waking thread %u; status is now %u\n", thread, pthread->state);
    return 0;
}

int pthread_yield() {
    printf("Yielding thread %u\n", current_thread);
    struct pthread_struct *pthread = pthread_get_struct_by_tid(current_thread);
    if(pthread == NULL) {
        errno = EINVAL;
        return EINVAL;
    }
    pthread->state = PTHREAD_STATE_RUNNABLE;
    int res = setjmp(pthread->hang_point);
    if(res) {
        return 0;
    } else {
        return pthread_schedule();
    }

}

int pthread_shutdown(pthread_t tid) {
    //Unmap the memory.  Pray really, really fast.
    struct pthread_struct *thread = pthread_get_struct_by_tid(tid);
    munmap(thread->stack_addr + 1 - thread->stack_size, thread->stack_size);
    fake_pthread_mutex_destroy(&(thread->join_mutex));
    fake_pthread_cond_destroy(&(thread->join_cond));

    thread->valid = 0;
    thread_mask &= ~(1 << (thread->tid));

    return 0; 
}

void fake_pthread_cleanup_push_generic(pthread_t tid, void  (*routine)(void *), const void *arg, int isSystem) { 
    struct pthread_struct *self = pthread_get_struct_by_tid(tid); 
    
    int *num_routines;
    void (**routines)(void *);
    void **args;

    if (self == NULL) {
        exit(-1); // was return -1, but errored because is a void function
    } 

    if(isSystem) {
        num_routines = &(self->num_system_cleanup_routines);
        routines = self->system_cleanup_routines;
        args = self->system_cleanup_args;
    } else {
        num_routines = &(self->num_user_cleanup_routines);
        routines = self->user_cleanup_routines;
        args = self->user_cleanup_args;
    }
    
    int idx = *num_routines;
    if(idx >= MAX_CLEANUP_ROUTINES) {
        errno = ENOMEM;
        return;
    }  
    *num_routines += 1;
    routines[idx] = routine;
    args[idx] = (void*)arg;
}

void fake_pthread_cleanup_pop_generic(pthread_t tid, int execute, int isSystem) {
    struct pthread_struct *self = pthread_get_struct_by_tid(tid); 

    int *num_routines;
    void (**routines)(void *);
    void **args;

    if (self == NULL) {
        exit(-1); // was return -1, but errored because is a void function
    } 

    if(isSystem) {
        num_routines = &(self->num_system_cleanup_routines);
        routines = self->system_cleanup_routines;
        args = self->system_cleanup_args;
    } else {
        num_routines = &(self->num_user_cleanup_routines);
        routines = self->user_cleanup_routines;
        args = self->user_cleanup_args;
    }

    int idx = (*num_routines) - 1;
    if(idx < 0) {
        errno = EINVAL;
        return;
    }
    void (*routine)(void *) = routines[idx];
    void *arg = args[idx];

    *num_routines -= 1;

    if(execute && routine != NULL) {
        routine(arg);
    }
}

int pthread_cleanup_remove_generic(pthread_t tid, void (*remove)(void *), int isSystem) {
    struct pthread_struct *self = pthread_get_struct_by_tid(tid);

    int *num_routines;
    void (**routines)(void *);
    void **args;

    if (self == NULL) {
        exit(-1); // was return -1, but errored because is a void function
    } 

    if(isSystem) {
        num_routines = &(self->num_system_cleanup_routines);
        routines = self->system_cleanup_routines;
        args = self->system_cleanup_args;
    } else {
        num_routines = &(self->num_user_cleanup_routines);
        routines = self->user_cleanup_routines;
        args = self->user_cleanup_args;
    }

    for(int i = 0; i < *num_routines; i++) {
        if(routines[i] == remove) {
            routines[i] = NULL;
            args[i] = NULL;
            return 0;
        }
    }
    return -1;
}

int pthread_cleanup_swaparg_generic(pthread_t tid, void (*swap)(void *), const void *new_arg, int isSystem) {
    struct pthread_struct *self = pthread_get_struct_by_tid(tid); 

    int *num_routines;
    void (**routines)(void *);
    void **args;

    if (self == NULL) {
        return -1;
    } 

    if(isSystem) {
        num_routines = &(self->num_system_cleanup_routines);
        routines = self->system_cleanup_routines;
        args = self->system_cleanup_args;
    } else {
        num_routines = &(self->num_user_cleanup_routines);
        routines = self->user_cleanup_routines;
        args = self->user_cleanup_args;
    }

    for(int i = 0; i < *num_routines; i++) {
        if(routines[i] == swap) {
            args[i] = (void*)new_arg;
            return 0;
        }
    }
    return -1;
}
