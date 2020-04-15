#!/bin/bash
#Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
find /usr/local/lib -name "libboost*.a" | while read -r lib
do
    FAKE_LIB=$(basename "$lib" | sed "s/^lib/fake_/;s/\.a//")
    INC_LIB=$(basename "$lib" | sed "s/^lib/include_/")
    echo "$FAKE_LIB"
    rm -rf "$FAKE_LIB"
done
