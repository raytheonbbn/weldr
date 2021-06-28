// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include "fake_socket.h"

struct socket_mgr mgr_of_socks;
struct fd_control sock_ctl;

INIT_WRAPPER(fake_socket) {
    init_sock_mgr();
    init_sock_ctl();
    return 0;
}

// Bypass function wrapping for socket()
extern int __real_socket(int domain, int type, int protocol); 

// Helper functions for defining fd_control struct.
int create_fd() {
    return __real_socket(AF_INET, SOCK_STREAM, 0);
}

void* create_impl_struct(int fd, void* args[]) {
    return (void*)create_fake_socket(fd, (int)(long)args[0], (int)(long)args[1], (int)(long)args[2]);
}

struct fake_filebuf *get_read_filebuf(fake_fd* ffd) {
    struct fake_socket* sock = (struct fake_socket*)ffd->impl_struct;
    return sock->in_buf;
}

struct fake_filebuf *get_write_filebuf(fake_fd* ffd) {
    struct fake_socket* sock = (struct fake_socket*)ffd->impl_struct;
    return sock->out_buf;
}

// Helper for managing the poll() function.
int get_poll_status(fake_fd* ffd) {
    
    struct fake_socket* sock = (struct fake_socket*)ffd->impl_struct;
    // For inet, we can use the mode to tell if we're ready to roll.
    // Once a connection is accepted, the FD is good for both
    // inbound and outbound comms.
    int out = 0;
    ssize_t in_avail;
    ssize_t out_avail;
    if(sock->mode == INBOUND || sock->mode == OUTBOUND) {
        in_avail = get_avail(sock->in_buf);
        out_avail = FD_BUFFER_SIZE - get_avail(sock->out_buf);
        printf("Connected socket %d (%ld, %ld):", ffd->fd, in_avail, out_avail);
        if(in_avail) {
            printf(" POLLIN");
            out |= POLLIN;
        }
        if(out_avail) {
            printf(" POLLOUT");
            out |= POLLOUT;
        }
        printf("\n");
    } else if(sock->mode == LISTEN) {
        if(sock->connecting_sock) {
            out |= POLLIN;
        }
    }
    return out;
}

int handle_ioctl(struct fake_fd *ffd, unsigned long request, int *ret, va_list args) {
    switch(request) {
#ifdef SIOCSIFMTU
    case(SIOCSIFMTU):
        // Set the MTU for an interface.
        // TODO: Always passing this through makes me uneasy.
        return FAKE_HANDLER_REAL;
#endif //SIOCSIFMTU
    default:
        return FAKE_HANDLER_SKIP;
    }
}

// Plug the helper functions into the function table struct.
int init_sock_ctl() {
    sock_ctl.create_fd = &create_fd;
    sock_ctl.create_impl_struct = &create_impl_struct;
    sock_ctl.get_read_filebuf = &get_read_filebuf;
    sock_ctl.get_write_filebuf = &get_write_filebuf;
    sock_ctl.get_poll_status = &get_poll_status;
    sock_ctl.handle_fcntl = NULL;
    sock_ctl.handle_ioctl = &handle_ioctl;
    sock_ctl.parent = &basic_fd_ctl;
    return 0;
}

// Init the global socket manager.
// This is really a very fancy way to control a large memory buffer.
int init_sock_mgr() {
    pthread_mutex_init(&(mgr_of_socks.bind_lock), NULL);
    pthread_cond_init(&(mgr_of_socks.bind_cond), NULL);

    pthread_rwlock_init(&(mgr_of_socks.sock_lock), NULL);

    mgr_of_socks.next_sock = 0;
    return 0;
}

// Make a new fake FD that behaves as a socket.
struct fake_socket* create_fake_socket(int fd, int domain, int type, int protocol) {
    pthread_rwlock_wrlock(&(mgr_of_socks.sock_lock));

    // If we've run out of fake file descriptors, return null.
    // The functions that call this will have to deal.
    if(mgr_of_socks.next_sock >= MAX_FAKE_SOCKS) {
        pthread_rwlock_unlock(&(mgr_of_socks.sock_lock));
        return NULL;
    }

    // Carve off a new socket control struct from our memory blob.
    //TODO: Handle freeing more gracefully to prevent fragmentation.
    struct fake_socket* out = mgr_of_socks.socks + mgr_of_socks.next_sock;
    mgr_of_socks.next_sock++;

    pthread_rwlock_unlock(&(mgr_of_socks.sock_lock));

    // Set our socket's data fields.    
    out->fd = fd;
    out->mode = NOT_INIT; 

    out->domain = domain;
    out->type = type;
    out->protocol = protocol;

    // Initialize the control semaphores.
    sem_init(&(out->connect_sem), 0, 1);
    sem_init(&(out->waiting_sem), 0, 0);
    sem_init(&(out->accept_sem), 0, 0);
    out->accepted_in_buf = NULL;
    out->accepted_out_buf = NULL;
    out->in_buf = NULL;
    out->out_buf = NULL;
    return out;
}

// Wait until another instance has bound a socket to which this address will connect.
struct fake_socket* wait_for_bound_addr(const struct sockaddr *addr) { 
    struct fake_socket* out = NULL;
    if(addr->sa_family != AF_INET && addr->sa_family != AF_INET6) {
        fprintf(stderr, "ABORT: asked to bind a non-INET socket: %d\n", addr->sa_family);
        exit(1);
    }
   
    // Spin as long as we're blocked.
    while(out == NULL) {
        // Lock off the manager's socket lock.
        pthread_rwlock_rdlock(&(mgr_of_socks.sock_lock));
        for(int i = 0; i < mgr_of_socks.next_sock; i++) {
            // Get the ith socket struct.
            struct fake_socket* tmp = mgr_of_socks.socks + i;
            struct sockaddr* bound_addr = &(tmp->addr);


            if(tmp->mode != LISTEN) {
                printf("Socket isn't listening; no match.\n");
                continue;
            }
            
            if(bound_addr->sa_family != addr->sa_family) {
                //printf("Different families; no match: expected %d but found %d\n", addr->sa_family, bound_addr->sa_family);
                printf("Different families, but assume they match.");
                out = tmp;
                break;
            }

            if(compare_sockaddrs(bound_addr, addr)) {
                out = tmp;
                break;
            } 
        }
        // If we didn't match, wait on the global bind condition variable.
        // When another thread binds, we'll have to scan again.
        if(out == NULL) {
            printf("Didn't find a match.  Waiting for someone to poke me.\n");
            pthread_mutex_lock(&(mgr_of_socks.bind_lock));
            pthread_rwlock_unlock(&(mgr_of_socks.sock_lock));
            pthread_cond_wait(&(mgr_of_socks.bind_cond), &(mgr_of_socks.bind_lock));
            pthread_mutex_unlock(&(mgr_of_socks.bind_lock));
        } else {
            pthread_rwlock_unlock(&(mgr_of_socks.sock_lock));
        }

    }
    // printf("We made it!\n");
    return out;
}

// Fake the listen() operation.
int fake_listen(int fd, int backlog) {
    // TODO: Actually use the backlog.
    struct fake_fd* ffd = get_fd_by_fd(fd);
    if(ffd == NULL) {
        return -1;
    }
    printf("Listening with %d\n", ffd->fd);
    struct fake_socket* sock = (struct fake_socket*)ffd->impl_struct;

    // Really, all listen does is tell the FD to get its butt in gear.
    // For our model, that's really minimal.
    sock->mode = LISTEN;
   
    return 0; 
}

// Fake the bind() operation.
int fake_bind(int fd, const struct sockaddr *addr) {
    struct fake_fd* ffd = get_fd_by_fd(fd);

    if(ffd == NULL) {
        return -1;
    }
    printf("Binding %d\n", ffd->fd);
    struct fake_socket* sock = (struct fake_socket*)ffd->impl_struct;
    pthread_rwlock_rdlock(&(mgr_of_socks.sock_lock));

    // Bind is composed of two steps.
    // 1) Set up the fake socket struct to show its binding.
    sock->mode = LISTEN;
    sock->addr = *addr;

    // 2) Poke the global bind condition variable to tell
    //    potential connectors that we're here.
    pthread_mutex_lock(&(mgr_of_socks.bind_lock));
    pthread_cond_signal(&(mgr_of_socks.bind_cond));
    pthread_rwlock_unlock(&(mgr_of_socks.sock_lock));
    pthread_mutex_unlock(&(mgr_of_socks.bind_lock));

    return 0;
}

// Fake the connect() operation.
int fake_connect(int fd, const struct sockaddr* addr) {
    struct fake_fd* my_fd = get_fd_by_fd(fd);
    if(my_fd == NULL) {
        return -1;
    }
    struct fake_socket* my_sock = (struct fake_socket*)my_fd->impl_struct;

    struct fake_socket* their_sock = wait_for_bound_addr(addr);

    // Wait for the accepting FD to have room for a connection.
    sem_wait(&(their_sock->connect_sem));
    their_sock->connecting_sock = my_sock;
    // tell the accepting socket that it's got a connection waiting.
    sem_post(&(their_sock->waiting_sem));

    // Wait for their fd to accept the connection.
    sem_wait(&(their_sock->accept_sem));
    struct fake_filebuf* in_buf = their_sock->accepted_in_buf;
    struct fake_filebuf* out_buf = their_sock->accepted_out_buf;

    // Clear their FD for the next connection.
    their_sock->connecting_sock = NULL;
    their_sock->accepted_in_buf = NULL;
    their_sock->accepted_out_buf = NULL;
    sem_post(&(their_sock->connect_sem));

    // Link the new socket into my fd.
    my_sock->mode = OUTBOUND;
    my_sock->in_buf = in_buf;
    my_sock->out_buf = out_buf;

    return 0;
}

// Fake the accept() operation.
int fake_accept(int fd) {
    printf("Accepting with %d\n", fd);
    // TODO: This only handles the basic accept() case.
    // Find the FD to accept on.
    struct fake_fd* ffd = get_fd_by_fd(fd);
    struct fake_socket* sock = (struct fake_socket*)ffd->impl_struct;

    // Block until we accept.
    sem_wait(&(sock->waiting_sem));
    // Create a new fake socket FD to carry the accepted connection.
    void* args[] = {(void*)(long)sock->domain, (void*)(long)sock->type, (void*)(long)sock->protocol};
    int out = create_fake_fd(&sock_ctl, args);
    struct fake_fd* out_fd = get_fd_by_fd(out);
    struct fake_socket* out_sock = (struct fake_socket*)out_fd->impl_struct;

    // Set up addresses for newly-bound sockets.
    out_sock->addr = sock->addr;
    sock->connecting_sock->addr = sock->addr;

    // Hook up comms between the two fake FDs.
    printf("Socket %d connected to socket %d, a proxy for %d\n", sock->connecting_sock->fd, out_sock->fd, sock->fd);
    struct fake_filebuf* in_buf = create_fake_filebuf();
    struct fake_filebuf* out_buf = create_fake_filebuf();
    out_sock->in_buf = in_buf;
    out_sock->out_buf = out_buf;
    out_sock->mode = INBOUND;
   
    // IMPORTANT: Swap the filebufs.  Our input is the output of the connecting sock, and vice versa.
    sock->accepted_in_buf = out_buf;
    sock->accepted_out_buf = in_buf;
    
    // Signal the server socket that we can accept again.
    sem_post(&(sock->accept_sem));

    return out;
}

// Partially fake the get_sockopt function.
int fake_getsockopt(int fd, int optname, void *optval, socklen_t *optlen) {
    struct fake_fd *ffd = get_fd_by_fd(fd);
    struct fake_socket *sock = (struct fake_socket*) ffd->impl_struct;
    struct timeval* timeout = NULL;
    char *iface = NULL;
    switch(optname) {
        case SO_ACCEPTCONN:
            // Test if the fake socket is accepting connections.
            *((int*)optval) = (sock->mode == LISTEN);
            *optlen = sizeof(int);
            return 0;
        case SO_BINDTODEVICE:
            // Get the device this socket is bound to.
            // For this model, we assume eth0
            // This looks dangerous, but the burden is on the user
            // to ensure optval points to a safe buffer.
            iface = (char*) optval;
            iface[0] = 'e';
            iface[1] = 't';
            iface[2] = 'h';
            iface[3] = '0';
            *optlen = 4;
            return 0;
        case SO_BROADCAST:
            // Test if the socket can broadcast.
            // Only valid for datagram sockets, 
            // so set this as false.
            *((int*)optval) = 0;
            *optlen = sizeof(int);
            return 0;
        case SO_DOMAIN:
            // Get the socket domain.
            *((int*)optval) = sock->domain;
            *optlen = sizeof(int);
            return 0;
        case SO_ERROR:
            // Get (and clear) the pending error.
            // TODO: Use this?
            *((int*)optval) = 0;
            *optlen = sizeof(int);
            return 0;
        case SO_PROTOCOL:
            // Get the protocol for this socket.
            *((int*)optval) = sock->protocol;
            *optlen = sizeof(int);
            return 0;
        case SO_RCVBUF:
            // Get the receiving buffer size for this socket.
            // Thankfully, ours our constant.
            *((int*)optval) = FD_BUFFER_SIZE;
            *optlen = sizeof(int);
            return 0;
        case SO_RCVBUFFORCE:
            // Get the receiving buffer size, with feeling.
            *((int*)optval) = FD_BUFFER_SIZE;
            *optlen = sizeof(int);
            return 0;
        case SO_RCVLOWAT:
            // Get the minimum bytes in the buffer before we trip a receive.
            *((int*)optval) = 1;
            *optlen = sizeof(int);
            return 0;
        case SO_SNDLOWAT:
            // Get the minimum bytes in the buffer before we trip a receive.
            *((int*)optval) = 1;
            *optlen = sizeof(int);
            return 0;
        case SO_RCVTIMEO:
            // Get the receive timeout for this socket.
            // We can't control for time with our concurrency model,
            // but we can report a nice comfortable value.
            timeout = ((struct timeval*)optval);
            timeout->tv_sec = 120;
            timeout->tv_usec = 0;
            *optlen = sizeof(struct timeval);
        case SO_SNDTIMEO:
            // Get the send timeout for this socket.
            // We can't control for time with our concurrency model,
            // but we can report a nice comfortable value.
            timeout = ((struct timeval*)optval);
            timeout->tv_sec = 120;
            timeout->tv_usec = 0;
            *optlen = sizeof(struct timeval);
        case SO_SNDBUF:
            // Get the send buffer size.
            // Thankfully, ours is constant.
            *((int*)optval) = FD_BUFFER_SIZE;
            *optlen = sizeof(int);
            return 0;
        case SO_SNDBUFFORCE:
            // Get the send buffer size, with feeling.
            *((int*)optval) = FD_BUFFER_SIZE;
            *optlen = sizeof(int);
            return 0;
        case SO_TYPE:
            // Get the socket type.
            *((int*)optval) = sock->type;
            *optlen = sizeof(int);
            return 0;
        default:
            // If the user gave us garbage, return an error.
            return -1;
    }
}

int fake_getsockname(int fd, struct sockaddr *addr, socklen_t *addrlen) {
    struct fake_fd *ffd = get_fd_by_fd(fd);

    if(ffd == NULL) {
        errno = ENOTSOCK;
        return -1;
    }
    struct fake_socket *sock = (struct fake_socket*) ffd->impl_struct;
    *addrlen = sizeof(struct sockaddr_in);
    *addr = sock->addr;
    return 0;
}

int fake_getpeername(int fd, struct sockaddr *addr, socklen_t *addrlen) {
    struct fake_fd *ffd = get_fd_by_fd(fd);

    if(ffd == NULL) {
        errno = ENOTSOCK;
        return -1;
    }
    struct fake_socket *sock = (struct fake_socket*) ffd->impl_struct;
    *addrlen = sizeof(struct sockaddr_in);
    *addr = sock->addr;
    return 0;
}
