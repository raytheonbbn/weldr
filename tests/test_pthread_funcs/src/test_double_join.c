// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define DELTA 2

void *thread_main(void *arg) {
    return arg + DELTA;
}

int main(int argc, char *argv[]) {
    long t1_in = 42;
    long t1_out = t1_in + DELTA;
    long t2_in = 24;
    long t2_out = t2_in + DELTA;


    pthread_t tid1;
    pthread_t tid2;

    if(pthread_create(&tid1, NULL, &thread_main, (void*)t1_in)) {
        perror("Failed to create thread 1");
        exit(1);
    }

    if(pthread_create(&tid2, NULL, &thread_main, (void*)t2_in)) {
        perror("Failed to create thread 2");
        exit(1);
    }

    void *data1;
    void *data2;

    if(pthread_join(tid1, &data1)) {
        perror("Failed to join thread 1");
        exit(1);
    }

    if(pthread_join(tid2, &data2)) {
        perror("Failed to join thread 2");
        exit(1);
    }

    if((long)data1 != t1_out) {
        fprintf(stderr, "Wrong result for thread 1; got %lu instead of %lu", (long)data1, t1_out);
        exit(1);
    }

    if((long)data2 != t2_out) {
        fprintf(stderr, "Wrong result for thread 2; got %lu instead of %lu", (long)data2, t2_out);
        exit(1);
    }

    return 0;
}
