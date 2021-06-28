// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include "hello.h"

int main(int argc, char *argv[]) {
    char str[50];
    fgets(str, 50, stdin);
    printf("You gave me %s\n", str);
    return 0;   
}
