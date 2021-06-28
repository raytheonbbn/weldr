# Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited
INCLUDES = -I ./include  -I ../include

include ../Makefile.common

vpath %.h include
vpath %.c src

GPATH=bin

OBJECTS = bin/redefine_streams.o

PIC_OBJECTS = bin/redefine_streams.pic.o

.PHONY : all
all : lib/libstubstdio.a lib/libstubstdio.so

lib/libstubstdio.a : $(OBJECTS)
	$(AR) rcs $@ $^

lib/libstubstdio.so : $(PIC_OBJECTS)
	$(CC) -shared $(CFLAGS) $^ -o $@

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
