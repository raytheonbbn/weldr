// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include "foo.h"

int baz(int a) {
    return a % 42;
}

int gorp(int a) {
    return a ^ 9;
}

int foo(int a, int b) {
    return baz(a) * gorp(b);
}

int bar(int a, int b) {
    return baz(a) * gorp(b);
}
