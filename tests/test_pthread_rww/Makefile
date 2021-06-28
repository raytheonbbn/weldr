# Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited
INCLUDES = -I ./include -I ../include

include ../../Makefile.common

CFLAGS=$(CFLAGS_SRC) -lpthread

vpath %.o bin
vpath %.h include
vpath %.c src

GPATH=bin

TEST_EXES = rw_counter_mutex_rww.exe

.PHONY : all
all : $(TEST_EXES)

%.exe : bin/%.o
	$(CC) $(CFLAGS) $< -o $@

bin/%.o : %.c bin
	$(CC) -c -I ../../../weldr/include $(CFLAGS) $< -o $@ -I ../include

bin:
	mkdir -p bin

.PHONY : clean
clean :
	rm -f bin/* 2> /dev/null
	rm -f *.exe 2> /dev/null
