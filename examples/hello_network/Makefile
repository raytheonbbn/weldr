#Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
INCLUDES = -I ./include -I ../include

include ../../Makefile.common

vpath %.h include
vpath %.c src

GPATH=bin

SPEAKER_OBJECTS = bin/speaker.o
LISTENER_OBJECTS = bin/listener.o

.PHONY : all
all : listener speaker

listener : $(LISTENER_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

speaker : $(SPEAKER_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

bin/%.o : %.c bin
	$(CC) -c $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

.PHONY : clean
clean :
	rm -f bin/* 2> /dev/null
	rm -f listener 2> /dev/null
	rm -f speaker 2> /dev/null
