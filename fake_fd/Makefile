#Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
INCLUDES = -I ./include -I ../include -I ../fake_pthread/include/public

include ../Makefile.common

vpath %.h include
vpath %.c src

GPATH=bin

WRAP_FLAGS=$(shell ../get_wrap_flags.sh wrap.conf) $(shell ../get_wrap_flags.sh ../fake_pthread/wrap.conf)

OBJECTS = bin/wrap_fileops.o bin/fake_fd.o
PIC_OBJECTS = bin/wrap_fileops.pic.o bin/fake_fd.pic.o

.PHONY : all
all : lib/libwrapfileops.so lib/libwrapfileops.a

lib/libwrapfileops.so : $(PIC_OBJECTS)
	$(CC) -shared $(CFLAGS) $(WRAP_FLAGS) -o $@ $^

lib/libwrapfileops.a : $(OBJECTS)
	$(AR) rcs $@ $^

bin/%.o : %.c bin lib
	$(CC) -c $(CFLAGS) $< -o $@

bin/%.pic.o : %.c bin lib
	$(CC) -c -fPIC $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

lib:
	mkdir -p lib

.PHONY : clean
clean :
	rm -f bin/* 2> /dev/null
	rm -f lib/* 2> /dev/null
