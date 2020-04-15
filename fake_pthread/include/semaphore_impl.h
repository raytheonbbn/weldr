// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#ifndef __SEMAPHORE_IMPL_H__
#define __SEMAPHORE_IMPL_H__

#include "pthread_shared.h"
#include "pthread_control.h"
#include "pthread_thread.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

//Definitions for posix semaphores.

#define SEM_MAX_WAITING_THREADS 32

struct sem_struct {
    char * name;
    int waiting_count;
    int waiting_off;
    pthread_t *waiting_threads;
    unsigned int value;
};

typedef struct sem_struct sem_t;

//Create or destroy an unnamed semaphore.
//If pshared is non-zero, the semaphore is visible from other processes.
int fake_sem_init(sem_t *sem, int pshared, unsigned int value);
int fake_sem_destroy(sem_t *sem);

//Open a handle to a named semaphore.
//oflags determines if the semaphore needs to be created.
//- O_CREAT: Open a new semaphore.  This takes two extra arguments:
//    - mode: a mode_t that defines the access control rules for the semaphore.
//    - value: an unsigned int defining the initial value of the semaphore.
//    The userID and groupID of the semaphore match the euid and egid of the process.
//    Once this call is complete, other processes can use fake_sem_open to get a handle
//    on this semaphore.
//    If a semaphore with this name exists, this flag does nothing.
//- O_EXCL: If this is set with O_CREAT, this call fails if a semaphore with this
//    name already exists.
sem_t *fake_sem_open(const char *name, int oflag, ...);

//Close a handle to a named semaphore.
//This just releases a process' access to the semaphore;
//the semaphore only gets destroyed when there are no
//open references, and fake_sem_unlink has been called
//since the last call to fake_sem_open with O_CREAT.
int fake_sem_close(sem_t *sem);

//Mark a named semaphore for destruction.
//As above, this will only destroy
//a semaphore once all references are closed.
int fake_sem_unlink(const char *name);

//Decrement the semaphore's value.
//If the value is zero, this will block
//until another thread posts.
int fake_sem_wait(sem_t *sem);

//Try to decrement the semaphore's value.
//This is the same as fake_sem_wait,
//except it returns an error instead of blocking
//on a depleted semaphore.
int fake_sem_trywait(sem_t *sem);

//Increment the semaphore's value.
//If there are threads blocked on the semaphore,
//this wakes them up and lets them acquire it.
int fake_sem_post(sem_t *sem);

//Get the semaphore's current value.
//If the semaphore is locked, this is zero...?
int fake_sem_getvalue(sem_t *sem, int *cur_value);
#endif
