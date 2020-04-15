// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "pthread_key.h"

unsigned int pthread_key_usage;
struct pthread_key_struct key_structs[MAX_PTHREAD_KEY_STRUCTS]; 

int init_pthread_key() {
    pthread_key_usage = 0;
}

struct pthread_key_struct *fake_pthread_key_get_struct_by_key(pthread_key_t key) {
    if(!(pthread_key_usage & (1 << key))) {
        return NULL;
    }
    return &(key_structs[key]);
}

int fake_pthread_key_create(pthread_key_t *key, void (*destructor)(void*)) {
    if(pthread_key_usage == 0xFFFFFFFF) { 
        errno = EAGAIN;
        return EAGAIN;
    }
    for(int i = 0; i < MAX_PTHREAD_KEY_STRUCTS; i++) {
        if(!(pthread_key_usage & (1 << i))) {
            *key = i;
            struct pthread_key_struct *ks = &(key_structs[i]);
            memset(ks->values, 0, sizeof(void*) * PTHREAD_MAX_THREADS);
            ks->destructor = destructor;
            pthread_key_usage |= (1 << i); 
            return 0;
        }
    }
    errno = EAGAIN;
    return EAGAIN;
}

int fake_pthread_key_delete(pthread_key_t key) {
    struct pthread_key_struct *ks = fake_pthread_key_get_struct_by_key(key);
    if(ks == NULL) {
        errno = EINVAL;
        return EINVAL; 
    }

    if(ks->destructor != NULL) {
        for(int i = 0; i < PTHREAD_MAX_THREADS; i++) {
            if(ks->values[i]) {
                pthread_cleanup_remove_generic(i, ks->destructor, 1);
            }
        }
    }
    return 0;
}

int fake_pthread_setspecific(pthread_key_t key, const void *value) {
    struct pthread_key_struct *ks = fake_pthread_key_get_struct_by_key(key);
    if(ks == NULL) {
        errno = EINVAL;
        return EINVAL; 
    }

    pthread_t tid = fake_pthread_self();
    ks->values[tid] = (void*)value;

    if(ks->destructor != NULL && pthread_cleanup_swaparg_generic(tid, ks->destructor, value, 1)) {
        fake_pthread_cleanup_push_generic(tid, ks->destructor, value, 1);
    }
    return 0;
}

void *fake_pthread_getspecific(pthread_key_t key) {
    struct pthread_key_struct *ks = fake_pthread_key_get_struct_by_key(key);
    if(ks == NULL) {
        errno = EINVAL;
        return NULL; 
    }
    pthread_t tid = fake_pthread_self();
    return ks->values[tid];
}
