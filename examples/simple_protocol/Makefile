#Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
INCLUDES = -I ./include -I ../include

include ../../Makefile.common

vpath %.h include
vpath %.c src

GPATH=bin

COMMON_OBJS=bin/common.o
CLIENT_OBJS=bin/client.o $(COMMON_OBJS) 
BROKER_OBJS=bin/broker.o $(COMMON_OBJS) 

all: client broker

client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

broker: $(BROKER_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

bin/%.o : %.c bin
	$(CC) -c $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

.PHONY : clean
clean :
	rm -f client 2> /dev/null
	rm -f broker 2> /dev/null
	rm -f bin/* 2> /dev/null
