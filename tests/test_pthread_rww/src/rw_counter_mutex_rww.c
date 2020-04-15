// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>

#define ACCESS_ONCE(counter) (*(volatile typeof(counter) *)&(counter))

int counter = 0;
int ready_1 = 0;
int ready_2 = 0;

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_2 = PTHREAD_COND_INITIALIZER;


void * reader(void * arg)
{    
    int i;
    pthread_rwlock_t *p = (pthread_rwlock_t *)arg;
    
    // Read lock
    pthread_rwlock_rdlock(p);
    
    // Read from resource
    printf(">>> counter value in reader() = %d\n", counter);

    // Acquire a lock
    printf("Locking mutex 1\n");
    pthread_mutex_lock(&mutex_1);

    if (!ready_1) {
        // Wating for cond_1 to resolve
        printf("Wating for cond_1 ....\n");
        pthread_cond_wait(&cond_1, &mutex_1);
    }

    // Unlock mutex
    printf("Unlocking mutex 1\n");
    pthread_mutex_unlock(&mutex_1);

    // Unlock read lock
    pthread_rwlock_unlock(p);
}

void * writer_1(void * arg)
{   
    int i;
    pthread_rwlock_t *p = (pthread_rwlock_t *)arg;

    // Write lock
    pthread_rwlock_wrlock(p);

    // Update resource aka WRITE
    ACCESS_ONCE(counter)++;
    printf("*** updated counter in writer_1() = %d\n", counter);

    // Acquire a lock
    printf("Locking mutex 2\n");
    pthread_mutex_lock(&mutex_2);

    if (!ready_2) {
        // Set wait condition
        printf("Wating for cond_2 ....\n");
        pthread_cond_wait(&cond_2, &mutex_2);
    }
    
    // Unlock mutex
    printf("Unlocking mutex 2\n");
    pthread_mutex_unlock(&mutex_2);
    
    // Unlock write lock
    pthread_rwlock_unlock(p);
}

void * writer_2(void * arg)
{   
    int i;
    pthread_rwlock_t *p = (pthread_rwlock_t *)arg;
    
    // Write lock
    pthread_rwlock_wrlock(p);

    // Update resource aka WRITE
    ACCESS_ONCE(counter)++;
    printf("*** updated counter in writer_2() = %d\n", counter);

    // Unlock write lock
    pthread_rwlock_unlock(p);
}

void * cond_1_release(void * arg)
{
    pthread_mutex_lock(&mutex_1);
    ready_1 = 1;
    pthread_mutex_unlock(&mutex_1);
    
    // Signal that mutex_1 can unlock
    printf("Signaling for cond_1 to resolve\n");
    pthread_cond_signal(&cond_1);
}

void * cond_2_release(void * arg)
{
    pthread_mutex_lock(&mutex_2);
    ready_2 = 1;
    pthread_mutex_unlock(&mutex_2);
    
    // Signal that mutex_2 can unlock
    printf("Signaling for cond_2 to resolve\n");
    pthread_cond_signal(&cond_2);
}

int main(int argc, char *argv[])
{
    pthread_t tid1, tid2, tid3, tid4, tid5;
    void *vp;

    if(pthread_create(&tid1, NULL, reader, &rwlock) != 0) {
        perror("pthread_create error");
        return 1;
    }

    if(pthread_create(&tid2, NULL, writer_1, &rwlock) != 0) {
        perror("pthread_create error");
        return 1;
    }

    if(pthread_create(&tid3, NULL, cond_1_release, &rwlock) != 0) {
        perror("pthread_create error");
        return 1;
    }

    if(pthread_create(&tid4, NULL, writer_2, &rwlock) != 0) {
        perror("pthread_create error");
        return 1;
    }

    if(pthread_create(&tid5, NULL, cond_2_release, &rwlock) != 0) {
        perror("pthread_create error");
        return 1;
    }

    pthread_join(tid1, &vp);
    pthread_join(tid2, &vp);
    pthread_join(tid3, &vp);
    pthread_join(tid4, &vp);
    pthread_join(tid5, &vp);

    printf("Counter in main() = %d\n", counter);
    pthread_rwlock_destroy(&rwlock);
    return 0;
}
