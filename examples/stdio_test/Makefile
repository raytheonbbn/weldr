# Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited
ROOTDIR := .

INCLUDES = -I $(ROOTDIR)/include -I ../include

include ../../Makefile.common

vpath %.h $(ROOTDIR)/include
vpath %.c $(ROOTDIR)/src

GPATH=$(ROOTDIR)/bin

OBJECTS = $(ROOTDIR)/bin/stdio_test.o

$(ROOTDIR)/stdio_test : $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(ROOTDIR)/bin/%.o : %.c $(ROOTDIR)/bin
	$(CC) -c $(CFLAGS) $< -o $@

$(ROOTDIR)/bin :
	mkdir -p $(ROOTDIR)/bin

.PHONY : clean
clean :
	rm $(ROOTDIR)/stdio_test 2> /dev/null
	rm $(ROOTDIR)/bin/* 2> /dev/null
