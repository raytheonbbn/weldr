// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited

#ifndef FAKE_FD_H
#define FAKE_FD_H

#include <pthread.h>
#include <semaphore.h>

#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "model.h"

#define MAX_FAKE_FDS 1024


typedef struct fake_fd fake_fd;

// Function table for implementation-specific routines.
//
// fake_fd is a generic library, intended to support different
// file-descriptor-based IPC mechanisms.
// This table defines the mechanism-specific behaviors for one mechanism.
struct fd_control {
    //Creation routine for dummy fd.
    int (*create_fd)();

    //Creation routine for implementation-specific struct.
    void *(*create_impl_struct)(int fd, void* args[]);

    //Getter for buffer struct when reading from an FFD.
    struct fake_filebuf *(*get_read_filebuf)(fake_fd*);

    //Getter for buffer struct when writing to an FFD.
    struct fake_filebuf *(*get_write_filebuf)(fake_fd*);

    //Getter for poll status
    int (*get_poll_status)(fake_fd*);

    //Handler for fcntl
    int (*handle_fcntl)(fake_fd* ffd, int cmd, int *ret, va_list args);

    //Handler for ioctl
    int (*handle_ioctl)(fake_fd* ffd, unsigned long request, int *ret, va_list args);

    //Parent pointer, for inheritance.
    struct fd_control *parent;
};

// Return values for handler functions.
#define FAKE_HANDLER_SUCCESS 0  // Handled the command. Return value is in ret.
#define FAKE_HANDLER_SKIP 1     // Didn't handle this command.  Try the parent.
#define FAKE_HANDLER_REAL 2     // Send this command to the real function.

// Default output value for handler-backed functions.
// Returned if nothing handled the command.
#define FAKE_HANDLER_DEFAULT 0

// Timeout in ms for polling on real file handles in our poll wrapper.
#define FAKE_POLL_TIMEOUT 100
#define FAKE_SELECT_TIMEOUT 100000

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

// Global manager struct for fake file descriptors and file buffers.
// This contains static arrays for each struct;
// ensures this library doesn't monkey with the heap.
// TODO: Allow recovery of free'd structs.
struct fd_mgr {
    pthread_rwlock_t fd_lock;   // rwlock for access to the fake fd array. 
    pthread_rwlock_t buf_lock;  // rwlock for access to the fake filebuf array.
    
    unsigned int next_fd;       // Next available fake_fd index.
    unsigned int next_buf;      // Next available fake_filebuf index.

    fake_fd fds[MAX_FAKE_FDS];              // fake file descriptor struct pool.
    struct fake_filebuf bufs[MAX_FAKE_FDS]; // fake file buffer struct pool.
};

//Manager management
int init_fd_mgr();

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

//Default handler functions.
int create_basic_fd();
int basic_handle_fcntl(fake_fd* ffd, int cmd, int *ret, va_list args);

//Root-level control structure; should only be inherited.
static struct fd_control basic_fd_ctl = {
    .create_fd = &create_basic_fd,
    .create_impl_struct = NULL,
    .get_read_filebuf = NULL,
    .get_write_filebuf = NULL,
    .get_poll_status = NULL,
    .handle_fcntl = &basic_handle_fcntl,
    .handle_ioctl = NULL,
    .parent = NULL
};
#endif
