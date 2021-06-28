# Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited
INCLUDES = -I ./include -I ../include

include ../../Makefile.common

vpath %.o bin
vpath %.h include
vpath %.c src

GPATH=bin

TEST_EXES = test_compile.exe test_single_join.exe test_single_mutex.exe test_single_cond.exe test_double_join.exe

.PHONY : all
all : $(TEST_EXES)

%.exe : bin/%.o
	$(CC) $(CFLAGS) $< -o $@

bin/%.o : %.c bin
	$(CC) -c $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

.PHONY : clean
clean :
	rm -f bin/* 2> /dev/null
