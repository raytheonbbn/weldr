# Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited
INCLUDES = -I ./include -I ../fake_fd/include  -I ../include

include ../Makefile.common

vpath %.h include
vpath %.c src

GPATH=bin

WRAP_FLAGS=$(shell ../get_wrap_flags.sh wrap.conf) $(shell ../get_wrap_flags.sh ../fake_pthread/wrap.conf)

OBJECTS = bin/wrap_network.o bin/fake_socket.o bin/sockaddr.o bin/inet_pton.o bin/inet_ntop.o bin/inet_addr.o bin/inet_ntoa.o
PIC_OBJECTS = bin/wrap_network.pic.o bin/fake_socket.pic.o bin/sockaddr.pic.o bin/inet_pton.pic.o bin/inet_ntop.pic.o bin/inet_addr.pic.o bin/inet_ntoa.pic.o

.PHONY : all
all : lib/libwrapnetwork.so lib/libwrapnetwork.a

lib/libwrapnetwork.so : $(PIC_OBJECTS)
	$(CC) -shared $(CFLAGS) $(WRAP_FLAGS) $^ -o $@

lib/libwrapnetwork.a : $(OBJECTS)
	$(AR) rcs $@ $^

bin/%.pic.o : %.c bin lib
	$(CC) -c -fPIC $(CFLAGS) $< -o $@

bin/%.o : %.c bin lib
	$(CC) -c $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

lib:
	mkdir -p lib

.PHONY : clean
clean :
	rm -f bin/* 2> /dev/null
	rm -f lib/* 2> /dev/null
