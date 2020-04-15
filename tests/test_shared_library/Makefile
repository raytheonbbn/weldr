#Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
include ../../Makefile.common
EXTRA_INCLUDES = -I ../../fake_pthread/include -I ../ -I ../include

.PHONY : all
all : test_plain test_wrapper

test_plain : src/test_pthread_plain.c
	$(CC) $(CFLAGS) -o $@ src/test_pthread_plain.c

test_wrapper : src/test_pthread_wrapper.c
	$(CC) $(CFLAGS) $(EXTRA_INCLUDES) -o $@ src/test_pthread_wrapper.c

.PHONY : clean
clean :
	rm -f test_plain 2> /dev/null
	rm -f test_wrapper 2> /dev/null
