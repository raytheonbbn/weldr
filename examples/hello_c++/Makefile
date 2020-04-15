#Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
INCLUDES = -I ./include -I ../include

include ../../Makefile.common

vpath %.h include
vpath %.cpp src

GPATH=bin

OBJECTS = bin/hello.o 

hello : $(OBJECTS)
	$(CXX) $(CFLAGS) $^ -L/usr/lib /usr/lib/gcc/x86_64-redhat-linux/8/libstdc++.a -o $@

bin/%.o : %.cpp bin
	$(CXX) -c $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

.PHONY : clean
clean :
	rm -f bin/* 2> /dev/null
	rm -f hello 2> /dev/null
