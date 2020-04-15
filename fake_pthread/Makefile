#Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
INCLUDES = -I ./include  -I ../include

include ../Makefile.common

vpath %.h include
vpath %.c src
vpath %.S src

GPATH=bin

OBJECTS = bin/fake_setjmp.oS bin/pthread_cancel.o bin/pthread_condattr.o bin/pthread_cond.o bin/pthread_key.o bin/pthread_launch.oS bin/pthread_mutexattr.o bin/pthread_mutex.o bin/pthread_once.o bin/pthread_rwlockattr.o bin/pthread_rwlock.o bin/pthread_thread_attr.o bin/pthread_thread.o bin/pthread_private.o bin/semaphore.o bin/wrap_pthread.o bin/wrap_semaphore.o bin/wrap_setjmp.o
PIC_OBJECTS = bin/fake_setjmp.pic.oS bin/pthread_cancel.pic.o bin/pthread_condattr.pic.o bin/pthread_cond.pic.o bin/pthread_key.pic.o bin/pthread_launch.pic.oS bin/pthread_mutexattr.pic.o bin/pthread_mutex.pic.o bin/pthread_once.pic.o bin/pthread_rwlockattr.pic.o bin/pthread_rwlock.pic.o bin/pthread_thread_attr.pic.o bin/pthread_thread.pic.o bin/pthread_private.pic.o bin/semaphore.pic.o bin/wrap_pthread.pic.o bin/wrap_semaphore.pic.o bin/wrap_setjmp.pic.o

.PHONY : all
all : lib/libwrappthread.so lib/libwrappthread.a

lib/libwrappthread.so : $(PIC_OBJECTS)
	$(CC) -shared $(CFLAGS) $(WRAP_FLAGS) $^ -o $@

lib/libwrappthread.a : $(OBJECTS)
	$(AR) rcs $@ $^

bin/%.pic.o : %.c bin lib
	$(CC) -c -fPIC $(CFLAGS) $< -o $@

bin/%.pic.oS : %.S bin lib
	$(CC) -c -fPIC $(CFLAGS_ASM) $< -o $@

bin/%.o : %.c bin lib
	$(CC) -c $(CFLAGS) $< -o $@

bin/%.oS : %.S bin lib
	$(CC) -c $(CFLAGS_ASM) $< -o $@

bin:
	mkdir -p bin

lib:
	mkdir -p lib

.PHONY : clean
clean :
	rm -f bin/* 2> /dev/null
	rm -f lib/* 2> /dev/null
