// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include "redefine_streams.h"

FILE* stdin = NULL;
FILE* stdout = NULL;
FILE* stderr = NULL;

char stdin_path[512];
char stdout_path[512];
char stderr_path[512];

void fake_stdio_logerr(const char * msg) {
    const char* err = strerror(errno);
    // We can't use stderr, since we overrode it.
    // However, we don't override the file descriptor,
    // so we can bang our message out with write()
    write(2, msg, strlen(msg));
    write(2, err, strlen(err));
    write(2, "\n", 2);
}

INIT_STUB(fake_stdio) {

    sprintf(stdin_path, "%s/%s_stdin", basedir, instance);
    sprintf(stdout_path, "%s/%s_stdout", basedir, instance);
    sprintf(stderr_path, "%s/%s_stderr", basedir, instance);

    int ret = 0;
    int tmp = 0;
    int c = 0;
    //Remove existing files
    if(ret = remove(stdin_path)) {
        if(errno != ENOENT) {
            fake_stdio_logerr("Failed to clear stdin path: ");
            return 2;
        } 
    }
    if(ret = remove(stdout_path)) {
        if(errno != ENOENT) {
            fake_stdio_logerr("Failed to clear stdout path: ");
            return 3;
        } 
    }
    if(ret = remove(stderr_path)) {
        if(errno != ENOENT) {
            fake_stdio_logerr("Failed to clear stderr path: ");
            return 4;
        } 
    }

    //Need to configure stdin separately; it should be a fifo, not a standard file.
    mode_t mode = 0666;
    if(mkfifo(stdin_path, mode)) {
        write(2, stdin_path, strlen(stdin_path));
        fake_stdio_logerr(" failed mkfifo(): ");
        return 5;
    }

    int stdin_fd = open(stdin_path, O_RDWR | O_CLOEXEC); // TODO isn't working with O_NONBLOCK
    if(!stdin_fd) {
        write(2, stdin_path, strlen(stdin_path));
        fake_stdio_logerr(" failed open(): ");
         
        return 6;
    }
 
    //Cheat way around fifo blocking; open for RW
    //Fortunately, this isn't a library meant to be developed against.    
    stdin=fdopen(stdin_fd, "rw"); 
    stdout=fopen(stdout_path, "w");
    stderr=fopen(stderr_path, "w");

    if(stdin == NULL) {
        write(2, stdin_path, strlen(stdin_path));
        fake_stdio_logerr(" failed fdopen(): ");
        return 7;
    }
    if(stdout == NULL) {
        write(2, stdout_path, strlen(stdout_path));
        fake_stdio_logerr(" failed fopen(): ");
        return 8;
    }
    if(stderr == NULL) {
        write(2, stderr_path, strlen(stderr_path));
        fake_stdio_logerr(" failed fopen(): ");
        return 9;
    }

    return 0;
}

int vprintf(const char *format, va_list arg) {
    return vfprintf(stdout, format, arg);
}

int printf(const char *format, ...) {
    
    va_list arg;
    va_start(arg, format);

    int out = vprintf(format, arg);
    
    va_end(arg);
    return out;
}

int scanf(const char* format, ...) {
    va_list arg;
    va_start(arg, format);
    
    int out = vfscanf(stdin, format, arg);
    
    va_end(arg);
    return out;
}

int getchar() {
    return getc(stdin);
}

char * gets(char *str) {
    //TODO: Hack.  Can I just throw an exception here, anyway?
    return fgets(str, 1024, stdin);
}

int putchar(int chr) {
    return fputc(chr, stdout);
}

int puts(const char *str) {
    return fputs(str, stdout);
}

void perror(const char *str) {
    char *err = strerror(errno);
    fprintf(stderr, "%s: %s\n", str, err);
}


FINI_STUB(fake_stdio) {
    if(stdin) {
        fclose(stdin);
        unlink(stdin_path);
    }
    if(stdout) {
        fclose(stdout);
    }
    if(stderr) {
        fclose(stderr); 
    }
}
