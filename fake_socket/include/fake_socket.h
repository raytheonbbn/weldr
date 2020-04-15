// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include <pthread.h>
#include <semaphore.h>

#include <errno.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <poll.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "fuse.h"
#include "fake_fd.h"
#include "sockaddr.h"

enum SOCKET_MODE {
    //Not tasked to do anything yet.
    NOT_INIT,
    //Tasked with listening for connections.
    LISTEN,
    //Writes to in, reads from out.
    INBOUND,
    //Writes to out, reads from in.
    OUTBOUND
};

struct fake_socket {
    //FD number, for printing.
    int fd;

    //Socket DTP triple
    int domain;
    int type;
    int protocol;
    
    //Address this is bound to, if applicable.
    struct sockaddr addr;
    
    //Socket requesting an acceptance.
    sem_t connect_sem;
    struct fake_socket* connecting_sock;
    sem_t waiting_sem;
    
    //Fake socket created for an acceptance.
    sem_t accept_sem;
    struct fake_filebuf* accepted_in_buf;
    struct fake_filebuf* accepted_out_buf;

    //Socket this is linked to, if any;
    enum SOCKET_MODE mode;
    struct fake_filebuf* in_buf;
    struct fake_filebuf* out_buf;
};


#define MAX_FAKE_SOCKS 128

struct socket_mgr {
    pthread_mutex_t bind_lock;
    pthread_cond_t bind_cond;

    pthread_rwlock_t sock_lock; 
    unsigned int next_sock;
    struct fake_socket socks[MAX_FAKE_SOCKS];
};

//Manager management
int init_sock_mgr();

struct fake_socket* wait_for_bound_ip(const char* ip, int port);

//Socket mangement
struct fake_socket* create_fake_socket(int fd, int domain, int type, int protocol);

//Network helpers
int fake_bind(int fd, const struct sockaddr *addr);
int fake_connect(int fd, const struct sockaddr *addr);
int fake_listen(int fd, int backlog);
int fake_accept(int fd);


int fake_getsockopt(int fd, int optname, void *optval, socklen_t *optlen);
int fake_getsockname(int fd, struct sockaddr *addr, socklen_t *addrlen);
int fake_getpeername(int fd, struct sockaddr *addr, socklen_t *addrlen);
