// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "redefine_streams.h"

FILE* stdin = NULL;
FILE* stdout = NULL;
FILE* stderr = NULL;

char stdin_path[512];
char stdout_path[512];
char stderr_path[512];

INIT_STUB(fake_stdio) {

    sprintf(stdin_path, "%s/%s_stdin", basedir, instance);
    sprintf(stdout_path, "%s/%s_stdout", basedir, instance);
    sprintf(stderr_path, "%s/%s_stderr", basedir, instance);

    int ret = 0;
    //Remove existing files
    if(ret = remove(stdin_path)) {
        if(errno != ENOENT) {
            return 2;
        } 
    }
    if(ret = remove(stdout_path)) {
        if(errno != ENOENT) {
            return 3;
        } 
    }
    if(ret = remove(stderr_path)) {
        if(errno != ENOENT) {
            return 4;
        } 
    }

    //Need to configure stdin separately; it should be a fifo, not a standard file.
    mode_t mode = 0666;
    if(mkfifo(stdin_path, mode)) {
        write(1, "Bad stdin path: '", strlen("Bad stdin path: '"));
        write(1, stdin_path, strlen(stdin_path));
        write(1, "'\n", 1);
        return 5;
    }

    int stdin_fd = open(stdin_path, O_RDWR | O_CLOEXEC); // TODO isn't working with O_NONBLOCK
    if(!stdin_fd) {
        write(1, "BAD\n", strlen("BAD\n"));
        return 6;
    }

    // TODO isn't working with NEW ADDITION code below
    /******************* NEW ADDITION ****************************/
    // To change access mode of file, must first use F_GETFL
    /*int prev_fl = fcntl(stdin_fd, F_GETFL, 0);

    if (prev_fl == -1) {
        perror("Error with fcntl() call, prv_fl\n");
        return 1;
    }
    
    // Change flag to be BLOCKING
    prev_fl &= ~O_NONBLOCK;

    // Update FIFO to be BLOCKING
    int cntl_check = fcntl(stdin_fd, F_SETFL, prev_fl);

    if(cntl_check == -1) {
        perror("Could not set FIFO to BLOCKING\n");
        return 1;
        }*/
    /***********************************************************/
    
    //Cheat way around fifo blocking; open for RW
    //Fortunately, this isn't a library meant to be developed against.    
    stdin=fdopen(stdin_fd, "rw"); 
    stdout=fopen(stdout_path, "w");
    stderr=fopen(stderr_path, "w");

    if(stdin == NULL) {
        return 7;
    }
    if(stdout == NULL) {
        return 8;
    }
    if(stderr == NULL) {
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
