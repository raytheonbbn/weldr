// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#ifndef __FUSE_BINARIES_H__
#define __FUSE_BINARIES_H__

#define INIT_STUB_CPP(lib) extern "C" int fuse_init_stub_ ## lib (const char *basedir, const char *instance) 
#define FINI_STUB_CPP(lib) extern "C" int fuse_fini_stub_ ## lib ()__attribute__((destructor)); \
    int fuse_fini_stub_## lib ()

#define INIT_STUB(lib) int fuse_init_stub_ ## lib (const char *basedir, const char *instance) 
#define FINI_STUB(lib) int fuse_fini_stub_ ## lib ()__attribute__((destructor)); \
    int fuse_fini_stub_## lib ()

#define INIT_WRAPPER(lib) int fuse_init_wrap_ ## lib ()
#define FINI_WRAPPER(lib) int fuse_fini_wrap_ ## lib ()__attribute__((destructor)); \
    int fuse_fini_wrap_## lib ()
#endif
