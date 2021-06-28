// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#ifndef __PTHREAD_KEY_H__
#define __PTHREAD_KEY_H__
#include "pthread_shared.h"
#include "pthread_thread.h"
#include <errno.h>
#include <string.h>

#define MAX_PTHREAD_KEY_STRUCTS 32

struct pthread_key_struct {
    void *values[PTHREAD_MAX_THREADS];
    void (*destructor)(void*);
};

typedef unsigned int  pthread_key_t;

//Library initializer.
int init_pthread_key();

//Create a shared value key.
//pthread_key_t objects are shared pointers to a thread-specific value;
//all threads can see key, but each thread has its own private value
//associated with key that only it can see.  When the key is created,
//this value is NULL.
//
//A key can have an associated destructor function assigned
//when create() is called.  When a thread exits, if destructor is
//not NULL and the thread's value is not NULL, it calls destructor
//on that value.
int fake_pthread_key_create(pthread_key_t *key, void (*destructor)(void*));

//Delete a shared value key.
//This removes the binding between the key and any stored values.
//It doesn't necessarily erase the values themselves,
//but it is now up to the application code to clean up any
//remaining values.  Any further attempt to use the key will result in undefined behavior.
//
//You can totally call delete() from inside a destructor.  This doesn't do anything special.
int fake_pthread_key_delete(pthread_key_t key);

//Get/set the thread-specific value bound to a given key.
//Not much more to it than that.  Calling setspecific()
//from inside a destructor is probably a bad idea,
//but this library won't prevent it.
int fake_pthread_setspecific(pthread_key_t key, const void *value);
void *fake_pthread_getspecific(pthread_key_t key);

//Design
//
//Each key is essentially a map from thread ID to value.
//The only problem is the destructor bit; there needs to be
//some mapping from thread back to keys in order to find
//which destructors to call.  This either needs to live
//in the thread itself, or in an external mapping
//that will need to be init'd.

#endif
