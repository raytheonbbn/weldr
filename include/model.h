// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#ifndef WELDR_MODEL_HOOKS_H
#define WELDR_MODEL_HOOKS_H

#define INIT_STUB_CPP(lib) extern "C" int weldr_init_stub_ ## lib (const char *basedir, const char *instance) 
#define FINI_STUB_CPP(lib) extern "C" int weldr_fini_stub_ ## lib ()__attribute__((destructor)); \
    int weldr_fini_stub_## lib ()

#define INIT_STUB(lib) int weldr_init_stub_ ## lib (const char *basedir, const char *instance) 
#define FINI_STUB(lib) int weldr_fini_stub_ ## lib ()__attribute__((destructor)); \
    int weldr_fini_stub_## lib ()

#define INIT_WRAPPER(lib) int weldr_init_wrap_ ## lib ()
#define FINI_WRAPPER(lib) int weldr_fini_wrap_ ## lib ()__attribute__((destructor)); \
    int weldr_fini_wrap_## lib ()
#endif
