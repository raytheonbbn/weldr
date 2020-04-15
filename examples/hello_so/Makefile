#Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
INCLUDES = -I ./include -I ../include

include ../../Makefile.common

vpath %.h include
vpath %.c src

GPATH=bin

hello : bin/hello.o lib/libfoo.so
	$(CC) $(CFLAGS) $< -Llib -lfoo -o $@

lib/libfoo.so : bin/foo.o lib
	$(CC) -shared $(CFLAGS) $< -o $@

bin/%.o : %.c bin
	$(CC) -c -fPIC $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

lib:
	mkdir -p lib

.PHONY : clean
clean :
	rm -f bin/* 2> /dev/null
	rm -f lib/* 2> /dev/null
	rm -f hello 2> /dev/null
