// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#define _GNU_SOURCE
#include "fake_fd.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include <poll.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//Function pointers to real library calls.
int __real_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int __real_pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timespec *timeout, const sigset_t *sigmask);

int __real_poll(struct pollfd *fds, nfds_t nfds, int timeout);
int __real_ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p, const sigset_t *sigmask);

int __real_fcntl(int fd, int cmd, ...);
int __real_ioctl(int fd, unsigned long request, ...);

int __real_close(int fd);
ssize_t __real_read(int fd, void *buf, size_t count);
ssize_t __real_write(int fd, const void *buf, size_t count);

// Function declarations of fake library calls
int __wrap_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int __wrap_pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timespec *timeout, const sigset_t *sigmask);

int __wrap_poll(struct pollfd *fds, nfds_t nfds, int timeout);
int __wrap_ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p, const sigset_t *sigmask);

int __wrap_fcntl(int fd, int cmd, ...);
int __wrap_ioctl(int fd, unsigned long request, ...);

int __wrap_close(int fd);
ssize_t __wrap_read(int fd, void *buf, size_t count);
ssize_t __wrap_write(int fd, const void *buf, size_t count);


int __wrap_fcntl(int fd, int cmd, ... /* arg */) {
    struct fake_fd *ffd = get_fd_by_fd(fd);
    
    va_list args;
    va_start(args, cmd);
    //fcntl takes one argument,
    //but its type changes based on cmd.
    //There is a vfcntl in the libc source, but it's not exposed.
    int isInt = 0;
    int i = 0;
    int isFlock = 0;
    struct flock* l = NULL;
    int isFOwnerEx = 0;
    struct f_owner_ex* f = NULL;
    int isUint32P = 0;
    uint32_t* u = NULL;
    int isModeled = 0;

    //These flags are a pain.  Historically, there are two reasons these haven't been defined in the past.
    //1) Linker issues.  I THINK the #define _GNU_SOURCE line fixes this.
    //2) Old versions of the kernel don't support all flags.
    //Solution is to use #ifdefs to detect which ones exist, and pray the linker doesn't barf on us again.
#ifdef F_DUPFD
    isInt |= (cmd == F_DUPFD);
#endif
#ifdef F_DUPFD_CLOEXEC
    isInt |= (cmd == F_DUPFD_CLOEXEC);
#endif
#ifdef F_SETFD
    isInt |= (cmd == F_SETFD);
#endif
#ifdef F_GETFL
    isModeled |= (cmd == F_GETFL);
#endif
#ifdef F_SETFL
    isInt |= (cmd == F_SETFL);
    isModeled |= (cmd == F_SETFL);
#endif
#ifdef F_SETOWN
    isInt |= (cmd == F_SETOWN);
#endif
#ifdef F_SETSIG
    isInt |= (cmd == F_SETSIG);
#endif
#ifdef F_SETLEAS
    isInt |= (cmd == F_SETLEAS);
#endif
#ifdef F_NOTIFY
    isInt |= (cmd == F_NOTIFY);
#endif
#ifdef F_SETPIPE_SZ
    isInt |= (cmd == F_SETPIPE_SZ);
#endif
    if(isInt) {
        i = va_arg(args, int);
    }

#ifdef F_STLK
    isFlock |= (cmd == F_STLK);
#endif
#ifdef F_SETLKW
    isFlock |= (cmd == F_SETLKW);
#endif
#ifdef F_GETLKW
    isFlock |= (cmd == F_GETLKW);
#endif
    if(isFlock) {
        l = va_arg(args, struct flock*);
    }

#ifdef F_GETOWN_EX
    isFOwnerEx |= (cmd == F_GETOWN_EX);
#endif
#ifdef F_SETOWN_EX
    isFOwnerEx |= (cmd == F_SETOWN_EX);
#endif
    if(isFOwnerEx) {
        f = va_arg(args, struct f_owner_ex*);
    }
    va_end(args);


    if(isModeled && ffd != NULL) {
        if(isInt) {
            return fake_fcntl_int(ffd, cmd, i);
        } else {
            return fake_fcntl_void(ffd, cmd);
        }
    } else if(ffd == NULL) {
        if(isInt) {
            return __real_fcntl(fd, cmd, i);
        } else if(isFlock) {
            return __real_fcntl(fd, cmd, l);
        } else if(isFOwnerEx) {
            return __real_fcntl(fd, cmd, f);
        } else if(isUint32P) {
            return __real_fcntl(fd, cmd, u);
        } else {
            return __real_fcntl(fd, cmd);
        }
    } else {
        return 0;
    }
}

int __wrap_ioctl(int fd, unsigned long request, ... /* arg */) {
    //Bypass if this is a fake FD.
    //I REALLY hope this isn't needed.
    struct fake_fd *ffd = get_fd_by_fd(fd);
    if(ffd != NULL) {
        return 0;
    }

    va_list args;
    va_start(args, request);
    //Again with the single-argument varargs.
    //Unlike fcntl, this isn't well-documented.
    //Hope like crazy that I don't kill my OS?
    char* arg = va_arg(args, char*);
    va_end(args);

    return __real_ioctl(fd, request, arg);
}

int __wrap_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    int out = 0;
    int real = 0;
    // Trip the scheduler to give other threads a voice.
    // We don't want to block, since we don't have anything
    // concrete to block on.
    if(pthread_yield()) {
        fprintf(stderr, "ABORT: failed to yield our thread.\n");
        exit(1);
    }
    for(int i = 0; i < FD_SETSIZE; i++) {
        struct fake_fd* ffd = get_fd_by_fd(i);
        short poll_flags = 0;
        if(ffd != NULL) {
            poll_flags = ffd->ctl->get_poll_status(ffd);
        }

        if(readfds != NULL && FD_ISSET(i, readfds)) {
            if(ffd != NULL) {
                printf("Read-Watching Fake FD: %d\n", i);
                if(poll_flags & POLLIN) {
                    FD_SET(i, readfds);
                    out++;
                } else {
                    FD_CLR(i, readfds);
                }
            } else {
                printf("Read-Watching Real FD: %d\n", i);
                real++;
            }
        }   
        if(writefds != NULL && FD_ISSET(i, writefds)) {
            if(ffd != NULL) {
                printf("Write-Watching Fake FD: %d\n", i);
                if(poll_flags & POLLOUT) {
                    FD_SET(i, writefds);
                    out++;
                } else {
                    FD_CLR(i, writefds);
                }
            } else {
                printf("Write-Watching Real FD: %d\n", i);
                real++;
            } 
        }
        if(exceptfds != NULL && FD_ISSET(i, exceptfds)) {
            if(ffd != NULL) {
                printf("Error-Watching Fake FD: %d\n", i);
                FD_CLR(i, exceptfds);
            } else {
                printf("Error-Watching Real FD: %d\n", i);
                real++;
            }
        }
    }
    if(real > 0) {
        fprintf(stderr, "ABORT: select can't handle real fds yet.\n");
        exit(1);
    }
    printf("We're watching %d file descriptors\n", out);
    
    return out;
}

int __wrap_pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timespec *timeout, const sigset_t *sigmask) {
    return __wrap_select(nfds, readfds, writefds, exceptfds, NULL);
}

int __wrap_poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    return __wrap_ppoll(fds, nfds, NULL, NULL);
}

int __wrap_ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p, const sigset_t *sigmask) {
    int count = 0;
    while(count == 0) {
        // Trip the scheduler to give other threads a voice.
        // We don't want to block, since we don't have anything
        // concrete to block on.
        if(pthread_yield()) {
            fprintf(stderr, "ABORT: failed to yield our thread.\n");
            exit(1);
        }
        // Iterate over the fds we're watching
        for(nfds_t i = 0; i < nfds; i++) {
            struct pollfd* f = (fds + i);
            struct fake_fd* ffd = get_fd_by_fd(f->fd);
            // If we have a real fd, abort.
            // FIXME: Integrate this.  I have ideas below.
            if(ffd == NULL) {
                fprintf(stderr, "ABORT: Handling a real fd.\n");
                exit(1);
            }
            // Collect the events relevant to our call.
            short fd_events = ffd->ctl->get_poll_status(ffd);
            printf("\tPoll Results: expected %x vs actual %x\n", f->events, fd_events);
            fd_events &= f->events;
            f->revents = fd_events;

            // If we had relevant, events, record the fact.
            if(fd_events) {
                count++;
            } 
        }
    }
    return count;
}

int __wrap_close(int fd) {
    struct fake_fd *ffd = get_fd_by_fd(fd);
    if (ffd != NULL) {
        return fake_close(fd);
    } else {
        return __real_close(fd);
    }
}

ssize_t __wrap_read(int fd, void *buf, size_t count) {
    struct fake_fd *ffd = get_fd_by_fd(fd);
    
    if(ffd != NULL) {
        return fake_read(fd, buf, count);
    } else {
        return __real_read(fd, buf, count);
    }
}

ssize_t __wrap_write(int fd, const void *buf, size_t count) {
    struct fake_fd *ffd = get_fd_by_fd(fd);
    
    if(ffd != NULL) {
        return fake_write(fd, buf, count);
    } else {
        return __real_write(fd, buf, count);
    }
}
