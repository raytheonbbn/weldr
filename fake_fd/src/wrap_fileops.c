// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#define _GNU_SOURCE
#include "fake_fd.h"
// The following includes can't be rolled into fake_fd.h
// They reference interfaces that interfere with other weldr models.
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

// Function declarations for vararg wrappers
int __fake_vfcntl(int fd, int cmd, va_list args);
int __fake_vioctl(int fd, unsigned long request, va_list args);

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


int __wrap_fcntl(int fd, int cmd, ... /* args */) {
    int out = 0;
    int res = 1;
    struct fake_fd *ffd = get_fd_by_fd(fd);
    struct fd_control *ctl;

    va_list args;
    va_start(args, cmd);

    if (ffd == NULL) {
        //If we're not a fake file descriptor,
        //call the real fcntl, via a varargs wrapper.
        out = __fake_vfcntl(fd, cmd, args);
    } else {
        // If we are fake, use the handler.
        ctl = ffd->ctl;
        do {
            if (ctl == NULL) {
                // We're out of handlers.
                out = FAKE_HANDLER_DEFAULT;
                break;
            }
            if (ctl->handle_fcntl) {
                res = ctl->handle_fcntl(ffd, cmd, &out, args);
                if (res == FAKE_HANDLER_SUCCESS) {
                    // Handler succeeded.
                    break;
                } else if (res == FAKE_HANDLER_REAL) {
                    // Handler delegated to the real function.
                    out = __fake_vfcntl(fd, cmd, args);
                    break;
                }
            }
            // Failed on this one; try the parent's handler.
            ctl = ctl->parent;
        } while(1);
    }
    va_end(args);
    return out;
}

int __fake_vfcntl(int fd, int cmd, va_list args) {
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
#ifdef F_SETFL
    isInt |= (cmd == F_SETFL);
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
}
int __wrap_ioctl(int fd, unsigned long request, ... /* args */) {
    int out = 0;
    int res = 1;
    struct fake_fd *ffd = get_fd_by_fd(fd);
    struct fd_control *ctl;

    va_list args;
    va_start(args, request);

    if (ffd == NULL) {
        //If we're not a fake file descriptor,
        //call the real ioctl, via a varargs wrapper.
        out = __fake_vioctl(fd, request, args);
    } else {
        // If we are fake, use the handler.
        ctl = ffd->ctl;
        do {
            if (ctl == NULL) {
                // We're out of handlers.
                out = FAKE_HANDLER_DEFAULT;
                break;
            }
            if (ctl->handle_ioctl) {
                res = ctl->handle_ioctl(ffd, request, &out, args);
                if (res == FAKE_HANDLER_SUCCESS) {
                    // Handler succeeded.
                    break;
                } else if (res == FAKE_HANDLER_REAL) {
                    // Handler delegated to the real function.
                    out = __fake_vioctl(fd, request, args);
                    break;
                }
            }
            // Failed on this one; try the parent's handler.
            ctl = ctl->parent;
        } while(1);
    }
    va_end(args);
    return out;
}

int __fake_vioctl(int fd, unsigned long request, va_list args) {
    //Again with the single-argument varargs.
    //Unlike fcntl, this isn't well-documented.
    //It seems to usually be a pointer?
    void *arg = va_arg(args, void*);
    return __real_ioctl(fd, request, arg);
}

int __wrap_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    int count = 0;
    int real = 0;

    fd_set real_readfds;
    fd_set real_writefds;
    fd_set real_exceptfds;
    struct timeval real_timeout = {
        .tv_sec = 0,
        .tv_usec = FAKE_SELECT_TIMEOUT
    };

    // Emulate blocking behavior by looping until count is positive,
    // but only if timeout is NULL.
    // TODO: Before, I had select without a loop.  Why?
    // I think it caused issues in mosquitto;
    // they use both poll() and select(), and the two were clashing.
    while(timeout == NULL && count == 0) {
        //Clear the real select parameters; we need to re-derive them.
        FD_ZERO(&real_readfds);
        FD_ZERO(&real_writefds);
        FD_ZERO(&real_exceptfds);
        real = 0;

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
        
            //If this is a fake file descriptor, get the poll status.
            if(ffd != NULL) {
                poll_flags = ffd->ctl->get_poll_status(ffd);
            }
            // Handle watching FDs for read events.
            if(readfds != NULL && FD_ISSET(i, readfds)) {
                if(ffd != NULL) {
                    // Check the fake fd's poll status to see
                    // if it has a read event pending.
                    printf("Read-Watching Fake FD: %d\n", i);
                    if(poll_flags & POLLIN) {
                        FD_SET(i, readfds);
                        count++;
                    } else {
                        FD_CLR(i, readfds);
                    }
                } else {
                    // Mark this FD for a real select.
                    printf("Read-Watching Real FD: %d\n", i);
                    FD_SET(i, &real_readfds);
                    // Have real store select's nfds parameter;
                    // highest-valued FD plus one.
                    real = i + 1;
                }
            }   
            // Handle watching FDs for write events.
            if(writefds != NULL && FD_ISSET(i, writefds)) {
                if(ffd != NULL) {
                    printf("Write-Watching Fake FD: %d\n", i);
                    if(poll_flags & POLLOUT) {
                        FD_SET(i, writefds);
                        count++;
                    } else {
                        FD_CLR(i, writefds);
                    }
                } else {
                    printf("Write-Watching Real FD: %d\n", i);
                    FD_SET(i, &real_writefds);
                    real = i + 1;
                }
            }
            // Handle watching FDs for errors.
            if(exceptfds != NULL && FD_ISSET(i, exceptfds)) {
                if(ffd != NULL) {
                    // Fake FDs won't have error conditions, so clear this bit.
                    printf("Error-Watching Fake FD: %d\n", i);
                    FD_CLR(i, exceptfds);
                } else {
                    printf("Error-Watching Real FD: %d\n", i);
                    FD_SET(i, &real_exceptfds);
                    real = i + 1;
                }
            }
        }
        if(real > 0) {
            // If we have real FDs, try selecting on them for a short while.
            real = select(real, &real_readfds, &real_writefds, &real_exceptfds, &real_timeout);
            if (real < 0) {
                // Error.  Let the user handle it.
                return -1;
            }
            // If it's not an error, 'real' will hold the number of events registered.
            count += real;
        }
    }
    
    return count;
}

int __wrap_pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timespec *timeout, const sigset_t *sigmask) {
    // Reformat the timeout for select()
    struct timeval to = {
        .tv_sec = timeout->tv_sec,
        .tv_usec = (timeout->tv_nsec / 1000l)
    };
    return __wrap_select(nfds, readfds, writefds, exceptfds, &to);
}

int __wrap_poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    return __wrap_ppoll(fds, nfds, NULL, NULL);
}

int __wrap_ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p, const sigset_t *sigmask) {
    int count = 0;
    int first = 1;
    struct pollfd real_fds[nfds];
    nfds_t real_idx[nfds];
    nfds_t n_real_fds = 0;

    // Iterate forever.  This ignores the timeout argument.
    // However, wall-clock time is rather nebulous in welded binaries.
    // TODO: Why do I ignore the timeout here, but not in select()?
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
            if(ffd == NULL) {
                //If we have a real FD, add it to the list on the first go-round.
                if(first && f->fd != -1) {
                    real_fds[n_real_fds].fd = f->fd;
                    real_fds[n_real_fds].events = f->events;
                    real_idx[n_real_fds] = i;
                    n_real_fds++;
                }
                continue;
            }
            // Collect the events relevant to our call.
            short fd_events = ffd->ctl->get_poll_status(ffd);
            fd_events &= f->events;
            f->revents = fd_events;

            // If we had relevant events, record the fact.
            if(fd_events) {
                printf("%p: fake fd %d had relevant events\n", fds, f->fd);
                count++;
            } 
        }
        if (n_real_fds > 0) {
            if(__real_poll(real_fds, n_real_fds, FAKE_POLL_TIMEOUT) < 0) {
                // Real poll encountered an error.  Let the user deal with it.
                return -1;
            }
            for(nfds_t i; i < n_real_fds; i++) {
                // Find the pollfd in the original array.
                struct pollfd *fake = (fds + real_idx[i]);
                // Update it with the returned events.
                fake->revents = real_fds[i].revents;
                // If we had results, increment count.
                if(fake->revents) {
                    printf("%p: real fd %d had relevant events\n", fds, fake->fd);
                    count++;
                }
            }
        }
    }
    printf("%p: %d relevant events total\n", fds, count);
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

// Handler function for fcntl.
// This relies on annoying header files, and can't be included in fake_fd.c
int basic_handle_fcntl(struct fake_fd* ffd, int cmd, int *ret, va_list args) {
    switch(cmd) {
#ifdef F_GETFL
        case F_GETFL:
            *ret = ffd->flags;
            return FAKE_HANDLER_SUCCESS;
#endif // F_GETFL
#ifdef F_SETFL
        case F_SETFL:
            ffd->flags = va_arg(args, int);
            *ret = 0; 
            return FAKE_HANDLER_SUCCESS;
#endif // F_SETFL
        default:
            return FAKE_HANDLER_SKIP;
    }
}
