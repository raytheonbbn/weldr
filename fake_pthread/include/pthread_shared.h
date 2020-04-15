// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#ifndef __PTHREAD_SHARED_H__
#define __PTHREAD_SHARED_H__

//Force-override pthread headers.
//...On Fedora
#define _BITS_PTHREADTYPES_COMMON_H
//...And on CentOS 6
#define _BITS_PTHREADTYPES_H



#define PTHREAD_MAX_THREADS 32
#define MAX_CLEANUP_ROUTINES 32

#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED 1

//The pthread_t object is an index into a global array of pthread structs.
typedef long unsigned int pthread_t;

#endif
