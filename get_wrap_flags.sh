#!/bin/bash
# Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited

WRAP_CONF=$1

OUT=""
while read l; do
    WRAP_CMD="-Wl,--wrap,$l"
    OUT="$OUT $WRAP_CMD"
done < $1
echo $OUT
