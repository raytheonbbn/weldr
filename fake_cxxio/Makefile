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
vpath %.cpp src

GPATH=bin

OBJECTS = bin/redefine_cxx_streams.o bin/allocate_cxx_streams.o
PIC_OBJECTS = bin/redefine_cxx_streams.pic.o bin/allocate_cxx_streams.pic.o

.PHONY : all
all : lib/libstubcxxio.a lib/libstubcxxio.so

lib/libstubcxxio.a : $(OBJECTS)
	$(AR) rcs $@ $^

lib/libstubcxxio.so : $(PIC_OBJECTS)
	$(CXX) -shared $(CFLAGS) $^ -o $@

bin/%.o : %.cpp bin lib
	$(CXX) -c $(CFLAGS) $< -o $@

bin/%.pic.o : %.cpp bin lib
	$(CXX) -c -fPIC $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

lib:
	mkdir -p lib

.PHONY : clean
clean :
	rm -f bin/* 2> /dev/null
	rm -f lib/* 2> /dev/null
