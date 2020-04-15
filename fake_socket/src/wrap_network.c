// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#define _GNU_SOURCE
#include "fake_socket.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
//#include <signal.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


//Function pointer struct.  Gotta hate extern :p
extern struct fd_control sock_ctl;

//Function pointers to real library calls.
int __real_socket(int domain, int type, int protocol);
int __real_socketpair(int domain, int type, int protocol, int sv[2]);
int __real_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int __real_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int __real_listen(int sockfd, int backlog);
int __real_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int __real_accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags);

int __real_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int __real_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int __real_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
int __real_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);



ssize_t __real_recv(int sockfd, void *buf, size_t len, int flags);
ssize_t __real_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *addrlen);
ssize_t __real_recvmsg(int sockfd, struct msghdr *msg, int flags);
ssize_t __real_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t __real_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t addrlen);
ssize_t __real_sendmsg(int sockfd, const struct msghdr *msg, int flags);


//Wrapper functions.
int __wrap_socket(int domain, int type, int protocol) {
    void* args[] = { (void*)(long)domain, (void*)(long)type, (void*)(long)protocol };

    int fd = create_fake_fd(&sock_ctl, args);
    printf("Created socket %d\n", fd);
    return fd;
}

int __wrap_socketpair(int domain, int type, int protocol, int sv[2]) {
    void* args[] = { (void*)(long)domain, (void*)(long)type, (void*)(long)protocol };
    sv[0] = create_fake_fd(&sock_ctl, args);
    sv[1] = create_fake_fd(&sock_ctl, args);

    fake_fd* a = get_fd_by_fd(sv[0]);
    fake_fd* b = get_fd_by_fd(sv[1]);

    struct fake_socket* sock_a = (struct fake_socket*)a->impl_struct;
    struct fake_socket* sock_b = (struct fake_socket*)b->impl_struct;

    struct fake_filebuf* in_buf = create_fake_filebuf(); 
    struct fake_filebuf* out_buf = create_fake_filebuf(); 

    sock_a->in_buf = in_buf;
    sock_b->out_buf = in_buf;
    
    sock_a->out_buf = out_buf;
    sock_b->in_buf = out_buf;

    sock_a->mode = INBOUND;
    sock_b->mode = OUTBOUND;

    printf("Created a linked pair of sockets, %d and %d\n", sv[0], sv[1]);

    return 0;
}

int __wrap_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    //For now, we assume that this is an inet socket.
    return fake_bind(sockfd, addr);
}

int __wrap_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return fake_connect(sockfd, addr);
}

int __wrap_listen(int sockfd, int backlog) {
    // I'm not 100% sure what this does in terms of networking...
    return fake_listen(sockfd, backlog);
}

int __wrap_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    //TODO: Figure out how to fake peer names.
    return fake_accept(sockfd);
}

int __wrap_accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) {
    //TODO: figure out how to fake peer names.
    //TODO: What do flags do?
    return fake_accept(sockfd);
}

int __wrap_getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    return fake_getsockname(sockfd, addr, addrlen);
}

int __wrap_getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    // This essentially returns the sockaddr struct
    // used to bind the socket in the first place.
    return fake_getsockname(sockfd, addr, addrlen);
}

int __wrap_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    //Fortunately, this is only ever called on sockets, which we control.
    //Unfortunately, some of these are valid for what we want to do.
    //FIXME: This ignores level, and assumes it to be socket.
    //Unfortunately, sockopt can work with protocols, too.
    switch(optname) {
        case SO_ACCEPTCONN:
        case SO_BINDTODEVICE:
        case SO_BROADCAST:
        case SO_DOMAIN:
        case SO_ERROR:
        case SO_PROTOCOL:
        case SO_RCVBUF:
        case SO_RCVBUFFORCE:
        case SO_RCVLOWAT:
        case SO_SNDLOWAT:
        case SO_RCVTIMEO:
        case SO_SNDTIMEO:
        case SO_SNDBUF:
        case SO_SNDBUFFORCE:
        case SO_TYPE:
            //These we actually model.
            return fake_getsockopt(sockfd, optname, optval, optlen);
            break;
        case SO_ATTACH_FILTER:
        case SO_LINGER:
            //We don't model this,
            //But it expects a pointer.
            *((void**)optval) = NULL;
            *(optlen) = sizeof(void*);
            return 0;
#ifdef SO_ATTACH_BPF
            //Defined as of kernel 3.19
        case SO_ATTACH_BPF:
#endif
#ifdef SO_REUSEPORT_CBPF
            //Defined as of kernel 3.19
        case SO_ATTACH_REUSEPORT_CBPF:
#endif
#ifdef SO_REUSEPORT_EBPF
            //Defined as of kernel 3.19
        case SO_ATTACH_REUSEPORT_EBPF:
#endif
        case SO_BSDCOMPAT:
        case SO_DEBUG:
        case SO_DONTROUTE:
#ifdef SO_INCOMING_CPU
            //defined as of kernel 3.19
        case SO_INCOMING_CPU:
#endif
        case SO_KEEPALIVE:
        case SO_LOCK_FILTER:
        case SO_MARK:
        case SO_OOBINLINE:
        case SO_PASSCRED:
        case SO_PEEK_OFF:
        case SO_PEERCRED:
        case SO_PRIORITY:
        case SO_REUSEADDR:
        case SO_REUSEPORT:
        case SO_RXQ_OVFL:
        case SO_TIMESTAMP:
        case SO_BUSY_POLL:
            // For many functions, ignore them.
            // This model doesn't support them.
            // AFAIK, these all take an int.
            *((int*)optval) = 0;
            *(optlen) = sizeof(int);
            return 0;
        default:
            return -1;            
    }
}

int __wrap_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    //This is also only called on socket FDs, so we don't need the O(n) garbage.
    //None of these are currently mutable.
    //For now, this does nothing.
    //FIXME: This ignores level, and assumes it to be socket.
    //Unfortunately, sockopt can work with protocols, too.
    switch(optname) {
        case SO_ACCEPTCONN:
        case SO_BINDTODEVICE:
        case SO_BROADCAST:
        case SO_DOMAIN:
        case SO_ERROR:
        case SO_PROTOCOL:
        case SO_RCVBUF:
        case SO_RCVBUFFORCE:
        case SO_RCVLOWAT:
        case SO_SNDLOWAT:
        case SO_RCVTIMEO:
        case SO_SNDTIMEO:
        case SO_SNDBUF:
        case SO_SNDBUFFORCE:
        case SO_TYPE:
            //The ones above this point are modeled for get.
        case SO_BUSY_POLL:
        case SO_ATTACH_FILTER:
        case SO_LINGER:
#ifdef SO_ATTACH_BPF
            //Defined as of kernel 3.19
        case SO_ATTACH_BPF:
#endif
#ifdef SO_REUSEPORT_CBPF
            //Defined as of kernel 3.19
        case SO_ATTACH_REUSEPORT_CBPF:
#endif
#ifdef SO_REUSEPORT_EBPF
            //Defined as of kernel 3.19
        case SO_ATTACH_REUSEPORT_EBPF:
#endif
        case SO_BSDCOMPAT:
        case SO_DEBUG:
        case SO_DONTROUTE:
#ifdef SO_INCOMING_CPU
            //defined as of kernel 3.19
        case SO_INCOMING_CPU:
#endif
        case SO_KEEPALIVE:
        case SO_LOCK_FILTER:
        case SO_MARK:
        case SO_OOBINLINE:
        case SO_PASSCRED:
        case SO_PEEK_OFF:
        case SO_PEERCRED:
        case SO_PRIORITY:
        case SO_REUSEADDR:
        case SO_REUSEPORT:
        case SO_RXQ_OVFL:
        case SO_TIMESTAMP:
            return 0;
        default:
            return -1;
    }
}

ssize_t __wrap_recv(int sockfd, void *buf, size_t len, int flags) {
    //TODO: This is WAY more complicated than this.
    //flags allows for a lot of features this can't model yet.
    return fake_read(sockfd, buf, len);
}

ssize_t __wrap_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *addrlen) {
    //TODO: This is WAY more complicated than this.
    //flags allows for a lot of features this can't model yet.
    //TODO: figure a way to get the source address out of the socket.
    return fake_read(sockfd, buf, len);
}

ssize_t __wrap_recvmsg(int sockfd, struct msghdr *msg, int flags) {
    //TODO: This is just evil.
    fprintf(stderr, "ABORT: recvmsg is not supported.");
    exit(1);
}

ssize_t __wrap_send(int sockfd, const void *buf, size_t len, int flags) {
    //TODO: This is WAY more complicated than this.
    //flags allows for a lot of features this can't model yet.
    return fake_write(sockfd, buf, len);
}

ssize_t __wrap_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t addrlen) {
    //TODO: This is WAY more complicated than this.
    //flags allows for a lot of features this can't model yet.
    //TODO: figure a way to get the source address out of the socket.
    return fake_write(sockfd, buf, len);
}

ssize_t __wrap_sendmsg(int sockfd, const struct msghdr *msg, int flags) {
    //TODO: This is just evil.
    fprintf(stderr, "ABORT: sendmsg is not supported.");
    exit(1);
}
