// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "fake_setjmp.h"

int __real__setjmp(jmp_buf env);
void __real__longjmp(jmp_buf env, int val);
int __real_setjmp(jmp_buf env);
void __real_longjmp(jmp_buf env, int val);

int __wrap__setjmp(jmp_buf env) {
    return fake_setjmp(env);
}

void __wrap__longjmp(jmp_buf env, int val) {
    fake_longjmp(env, val);
}

int __wrap_setjmp(jmp_buf env) {
    return fake_setjmp(env);
}

void __wrap_longjmp(jmp_buf env, int val) {
    fake_longjmp(env, val);
}
