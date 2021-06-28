#!/bin/bash
# Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited
make clean all
./test_plain > plain.out
./test_wrapper > wrapper.out
diff plain.out wrapper.out
