# Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited
INCLUDES = -I ./include -I ../include

include ../../Makefile.common

vpath %.h include
vpath %.cpp src

GPATH=bin

OBJECTS = bin/main.o bin/thing_one.o bin/thing_two.o

hello : $(OBJECTS)
	$(CXX) $(CFLAGS) $^ -o $@

bin/%.o : %.cpp bin
	$(CXX) -c $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

.PHONY : clean
clean :
	rm -f bin/* 2> /dev/null
	rm -f hello 2> /dev/null
