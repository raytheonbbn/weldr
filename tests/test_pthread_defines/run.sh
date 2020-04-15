#!/bin/bash
#Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
make clean all
./test_plain > plain.out
./test_wrapper > wrapper.out
diff plain.out wrapper.out
