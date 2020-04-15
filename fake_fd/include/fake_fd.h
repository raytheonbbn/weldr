// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration

#include <pthread.h>
#include <semaphore.h>

#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "fuse.h"

#define MAX_FAKE_FDS 1024


typedef struct fake_fd fake_fd;

struct fd_control {
    //Creation routine.
    void *(*create_impl_struct)(int, void* args[]);
    //Getter for buffer structs
    struct fake_filebuf *(*get_filebuf)(fake_fd*, int is_read);
    //Getter for poll status
    int (*get_poll_status)(fake_fd*);
};

struct fake_fd {
    //FD that this has taken over.
    int fd;
    int closed;
    int flags;

    //Generic plugin for type-specific data.
    void* impl_struct;
    struct fd_control* ctl;

};

#define FD_BUFFER_SIZE 1024

struct fake_filebuf {
    char buf[FD_BUFFER_SIZE];
    ssize_t off;
    ssize_t len;
    pthread_mutex_t lock;
    pthread_cond_t read_cond;
};

struct fd_mgr {
    pthread_rwlock_t fd_lock; 
    pthread_rwlock_t buf_lock;
    
    unsigned int next_fd;
    unsigned int next_buf;

    fake_fd fds[MAX_FAKE_FDS];
    struct fake_filebuf bufs[MAX_FAKE_FDS]; 
};

//Manager management
int init_fd_mgr();
int init_sock_ctl();

//FD management
int create_fake_fd(struct fd_control* ctl, void* impl_params[]);
fake_fd* get_fd_by_fd(int fd);
fake_fd* remove_fd_by_fd(int fd);

//File Buf management.
struct fake_filebuf* create_fake_filebuf();
ssize_t get_avail(struct fake_filebuf* fb);

//Override functions.
int fake_close(int fd);
int fake_read(int fd, char* buf, ssize_t size);
int fake_write(int fd, const char* buf, ssize_t size);
int fake_flush(int fd);
int fake_fcntl_int(struct fake_fd* ffd, int cmd, int arg);
int fake_fcntl_void(struct fake_fd* ffd, int cmd);
