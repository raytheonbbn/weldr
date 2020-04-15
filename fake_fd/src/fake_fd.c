// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "fake_fd.h"

struct fd_mgr mgr_of_fds;

INIT_WRAPPER(fake_fd) {
    return init_fd_mgr();
}

int init_fd_mgr() {
    pthread_rwlock_init(&(mgr_of_fds.fd_lock), NULL);
    pthread_rwlock_init(&(mgr_of_fds.buf_lock), NULL);

    mgr_of_fds.next_fd = 0;
    mgr_of_fds.next_buf = 0;
    return 0;
}

int create_fake_fd(struct fd_control* ctl,  void* impl_params[]) {
    int fd = dup(1);
    
    fake_fd* ffd = NULL;

    pthread_rwlock_wrlock(&(mgr_of_fds.fd_lock));

    if(mgr_of_fds.next_fd >= MAX_FAKE_FDS) {
        //TODO: Find some way to reuse FDs.
        pthread_rwlock_unlock(&(mgr_of_fds.fd_lock));
        return -1;
    }
    ffd = mgr_of_fds.fds + mgr_of_fds.next_fd;
    mgr_of_fds.next_fd++;

    ffd->fd = fd;
    ffd->impl_struct = ctl->create_impl_struct(fd, impl_params);
    ffd->ctl = ctl;
    ffd->closed = 0;

    pthread_rwlock_unlock(&(mgr_of_fds.fd_lock));


    return fd;
}

fake_fd* get_fd_by_fd(int fd) {
    fake_fd* out = NULL;
    pthread_rwlock_rdlock(&(mgr_of_fds.fd_lock));

    for(int i = 0; i < mgr_of_fds.next_fd; i++) {
        out = mgr_of_fds.fds + i;
        if(out->fd == fd) {
            break;
        } else {
            out = NULL;
        }
    }

    pthread_rwlock_unlock(&(mgr_of_fds.fd_lock));
    return out;
}

fake_fd* remove_fd_by_fd(int fd) { 
    return NULL;
}

struct fake_filebuf* create_fake_filebuf() {
    pthread_rwlock_wrlock(&(mgr_of_fds.buf_lock));

    if(mgr_of_fds.next_buf >= MAX_FAKE_FDS) {
        pthread_rwlock_unlock(&(mgr_of_fds.buf_lock));
        return NULL;
    }

    struct fake_filebuf* out = mgr_of_fds.bufs + mgr_of_fds.next_buf;
    mgr_of_fds.next_buf++;

    pthread_rwlock_unlock(&(mgr_of_fds.buf_lock));

    out->off = 0;
    out->len = 0;
    pthread_mutex_init(&(out->lock), NULL);
    pthread_cond_init(&(out->read_cond), NULL);

    return out;
}

ssize_t get_avail(struct fake_filebuf* fb) {
    ssize_t out = 0;
    pthread_mutex_lock(&(fb->lock));
    out = fb->len;
    pthread_mutex_unlock(&(fb->lock));
    return out;
}

//Essentially a copy of memcpy that we can search for.
void input_target(char* buf, const char* source, size_t size) {
    memcpy(buf, source, size); 
}

void output_target(const char* buf, char* target, size_t size) {
    memcpy(target, buf, size);
}

int fake_read(int fd, char* buf, ssize_t size) {

    fake_fd* ffd = get_fd_by_fd(fd);
    
    struct fake_filebuf* fb = ffd->ctl->get_filebuf(ffd, 1);

    // We need to keep anyone else from mucking with the buffer.
    pthread_mutex_lock(&(fb->lock));
     
    if(ffd->closed) {
        //printf("FD %d is closed\n", fd);
        //Turning this into a nuke; we should never hit this.
        exit(-1);
    }
    
    if(fb->len == 0) {
        //Wait for something to be readable.
        pthread_cond_wait(&(fb->read_cond), &(fb->lock));
    }
    
    if(ffd->closed) {
        //printf("FD %d is closed\n", fd);
        //Turning this into a nuke; we should never hit this.
        exit(-1);
    }

    // Compute the appropriate read length.
    ssize_t to_read = size;
    if(fb->len < size) {
        to_read = fb->len;
    }

    ssize_t first_off = fb->off;
    ssize_t first_len = to_read;
    ssize_t second_len = 0;

    // We need to circle the buffer.
    if(first_off + to_read > FD_BUFFER_SIZE) {
        // First read offset stays the same.
        // First read length is now equal to the amount left in the buffer.
        first_len = FD_BUFFER_SIZE - first_off;
        // Second read offset is always zero.
        // Second read length is the length of the spillover.
        second_len = first_off + to_read - FD_BUFFER_SIZE;
    }
    
    // Copy the data out of the buffer.
    // TODO: Two calls to input_target will look weird.  Live with it.
    input_target(buf, fb->buf + first_off, first_len);
    if(second_len > 0) {
        input_target(buf + first_len, fb->buf, second_len);
    }

    // Update the offset and length
    fb->off = (fb->off + to_read) % FD_BUFFER_SIZE;
    fb->len -= to_read;

    // If we left some leftovers, signal other readers, if any.
    if(fb->len > 0) {
        pthread_cond_signal(&(fb->read_cond));
    }

    // Let someone else play now.
    pthread_mutex_unlock(&(fb->lock));

    printf("Read %d bytes from %d [Buf %p]\n", to_read, fd, fb);

    return to_read;
}

int fake_write(int fd, const char* buf, ssize_t size) {

    fake_fd* ffd = get_fd_by_fd(fd);
    struct fake_filebuf* fb = ffd->ctl->get_filebuf(ffd, 0);

    // We need to keep anyone else from mucking with the buffer.
    pthread_mutex_lock(&(fb->lock));

    if(ffd->closed) {
        //printf("FD %d is closed\n", fd);
        //Turning this into a nuke; we should never hit this.
        exit(-1);
    }

    // Compute the appropriate write length.
    ssize_t to_write = size;
    if(to_write > FD_BUFFER_SIZE - fb->len) {
        to_write = FD_BUFFER_SIZE - fb->len;
    }

    ssize_t first_off = fb->off + fb->len;
    ssize_t first_len = to_write;
    ssize_t second_off = 0;
    ssize_t second_len = 0;

    // We need to circle the buffer.
    if(first_off + to_write > FD_BUFFER_SIZE) {
        // First write offset stays the same.
        // First write length is now equal to the amount of space left in the buffer.
        first_len = FD_BUFFER_SIZE - first_off;
        // Second write offset is always zero.
        // Second write length is the length remaining after truncating the first.
        second_len =  to_write - first_len;
    }
    
    // Copy the data into the buffer.
    // TODO: Two calls to input_target will look weird.  Live with it.
    output_target(buf, fb->buf + first_off, first_len);
    if(second_len > 0) {
        output_target(buf + first_len, fb->buf, second_len);
    } 

    // Update the length
    fb->len += to_write;

    // Let waiting readers know they can read.
    pthread_cond_signal(&(fb->read_cond));

    pthread_mutex_unlock(&(fb->lock));

    printf("Wrote %d bytes to %d [Buf %p]\n", to_write, fd, fb);

    return to_write;
}

int fake_close(int fd) {
    fake_fd* ffd = get_fd_by_fd(fd);
    ffd->closed=1;
    struct fake_filebuf* buf_a = ffd->ctl->get_filebuf(ffd, 0);
    struct fake_filebuf* buf_b = ffd->ctl->get_filebuf(ffd, 1);
    pthread_mutex_unlock(&(buf_a->lock));
    if(buf_b != NULL && buf_a != buf_b) {
        pthread_mutex_unlock(&(buf_b->lock));
    }

    return 0;
}


int fake_flush(int fd) {
    //Do nothing; our buffer always "flushes",
    //because it's shared between both "processes".
    return 0;
}

int fake_fcntl_int(struct fake_fd* ffd, int cmd, int arg) {
    switch(cmd) {
#ifdef F_SETFL
        case F_SETFL:
            printf("Setting flags of fd %d to %x", ffd->fd, arg);
            ffd->flags = arg;
            return 0;
#endif
        default:
            return 0;
    }
}

int fake_fcntl_void(struct fake_fd* ffd, int cmd) {
    switch(cmd) {
#ifdef F_SETFL
        case F_SETFL:
            return ffd->flags;
            break;
#endif
        default:
            return 0;
    }
}
