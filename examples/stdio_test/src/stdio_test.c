// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "stdio_test.h"

int main(int argc, char *argv[]) {
    printf("stdout test 1\n");

    fprintf(stdout, "stdout test 2\n");

    fputs("stdout test 3\n", stdout);

    fputs("stdout test ", stderr);
    fputc('4', stdout);
    fputc('\n', stdout);

    fputs("stdout test ", stdout);
    putc('5', stdout);
    putc('\n', stdout); 

    fputs("stdout test ", stdout);
    putchar('6');
    putchar('\n');

    fflush(stdout);

    fprintf(stderr, "stderr test 1\n");

    fputs("stderr test 2\n", stderr);

    fputs("stderr test ", stderr);
    fputc('3', stderr);
    fputc('\n', stderr);

    fputs("stderr test ", stderr);
    putc('4', stderr);
    putc('\n', stderr); 

    fflush(stderr);
}
