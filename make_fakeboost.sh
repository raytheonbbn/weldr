#!/bin/bash
#Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
find /usr/local/lib -name "libboost*.a" | while read -r lib
do
    FAKE_LIB=$(basename "$lib" | sed "s/^lib/fake_/;s/\.a//")
    INC_LIB=$(basename "$lib" | sed "s/^lib/include_/")
    echo "$FAKE_LIB"
    rm -rf "$FAKE_LIB"
    mkdir "$FAKE_LIB"
    pushd "$FAKE_LIB"
        echo ".PHONY : all" >> "Makefile"
        echo "all : lib/$INC_LIB include.conf deps.conf" >> "Makefile"
        echo "" >> "Makefile"
        echo "deps.conf :" >> "Makefile"
        echo -e "\techo '../fake_boost_system' > deps.conf" >> "Makefile"
        echo "include.conf :" >> "Makefile"
        echo -e "\ttouch include.conf" >> "Makefile"
        echo "" >> "Makefile"
        echo "lib/$INC_LIB : $lib lib" >> "Makefile"
        echo -e "\tcp $lib ./lib/$INC_LIB" >> "Makefile"
        echo "" >> "Makefile"
        echo "lib :" >> "Makefile"
        echo -e "\tmkdir -p lib" >> "Makefile"
        echo "" >> "Makefile"
        echo ".PHONY : clean" >> "Makefile"
        echo "clean :" >> "Makefile"
        echo -e "\trm lib/* 2> /dev/null" >> "Makefile"
        make
    popd
    
done
