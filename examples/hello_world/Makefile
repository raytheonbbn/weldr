#Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
INCLUDES = -I ./include -I ../include

include ../../Makefile.common

vpath %.h include
vpath %.c src

GPATH=bin

OBJECTS = bin/hello.o 

hello : $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

bin/%.o : %.c bin
	$(CC) -c $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

.PHONY : clean
clean :
	rm -f bin/* 2> /dev/null
	rm -f hello 2> /dev/null
