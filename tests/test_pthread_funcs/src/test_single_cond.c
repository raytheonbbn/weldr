// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

pthread_cond_t cond;
pthread_mutex_t mutex;

#define OLD_VALUE 24
#define NEW_VALUE 42

void *thread_main(void *arg) {
    if(pthread_mutex_lock(&mutex)) {
        perror("Failed to lock mutex in thread.");
        exit(1);
    }

    if(pthread_cond_wait(&cond, &mutex)) {
        perror("Failed to block on cond in thread.");
        exit(1);
    }

    if(*((int*)arg) != OLD_VALUE) {
        fprintf(stderr, "Incorrect value after cond release: %d instead of %d\n", *((int*)arg), NEW_VALUE);
        exit(1);
    }

    *((int*)arg) = NEW_VALUE;

    if(pthread_mutex_unlock(&mutex)) {
        perror("Failed to unlock mutex in thread.");
        exit(1);
    }
    return NULL;
}

int main(int argc, char *argv) {
    pthread_t tid;

    volatile int value = 0;

    if(pthread_mutex_init(&mutex, NULL)) {
        perror("Failed to init mutex");
        exit(1);
    }

    if(pthread_cond_init(&cond, NULL)) {
        perror("Failed to init cond");
        exit(1);
    }

    if(pthread_create(&tid, NULL, &thread_main, &value)) {
        perror("Failed to create thread");
        exit(1);
    }
    
    if(pthread_mutex_lock(&mutex)) {
        perror("Failed to lock mutex in main");
        exit(1);
    }

    value = OLD_VALUE;

    if(pthread_cond_signal(&cond)) {
        perror("Failed to signal cond in main");
        exit(1);
    }
    
    if(pthread_mutex_unlock(&mutex)) {
        perror("Failed to unlock mutex in main");
        exit(1);
    }

    if(pthread_join(tid, NULL)) {
        perror("Failed to join thread");
        exit(1);
    }
    
    if(value != NEW_VALUE) {
        fprintf(stderr, "Incorrect value; %d instead of %d\n", value, NEW_VALUE);
        exit(1);
    }

    printf("Test passed\n");
    return 0;
}
