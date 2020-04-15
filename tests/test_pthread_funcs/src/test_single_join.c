// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

void *thread_main(void *arg) {
    return (void*)42;
}

int main(int argc, char *argv) {
    pthread_t tid;
    if(pthread_create(&tid, NULL, &thread_main, NULL)) {
        perror("Failed to create thread");
        exit(1);
    }

    void *data;

    if(pthread_join(tid, &data)) {
        perror("Failed to join thread");
        exit(1);
    }

    if((long)data != 42) {
        perror("Incorrect return");
        exit(1);
    }
    printf("Test passed\n");
    return 0;
}
