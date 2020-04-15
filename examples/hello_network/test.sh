#!/bin/bash
#Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
./listener &
PID=$!
sleep 1

./speaker
kill $PID
