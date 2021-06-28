#!/bin/bash
# Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited

TOTAL=0
PASSED=0

record_pass() {
    TOTAL=$((TOTAL+1))
    if [ "$2" == "0" ]; then
        PASSED=$((PASSED+1))
        echo ""
        echo "********$1 PASSED********"
        echo ""
    else
        echo ""
        echo "********$1 FAILED($2)********"
        echo ""
    fi
}

run_weldr() {
    PRJ_PATH=$1
    TST_NAME=$2
    SYS_PATH=$3
    OUT=0

    DIR_PATH="$PRJ_PATH/$TST_NAME.fuse_out"
    rm -rf $DIR_PATH
    pushd ../../fuse_binaries > /dev/null
    ./fuse.sh -t $DIR_PATH -s $SYS_PATH -l ../fake_pthread $PRJ_PATH
    if [ "$?" != "0" ]; then
        OUT=1
    else
        $DIR_PATH/run_me
        OUT=$?
    fi
    popd > /dev/null
    return OUT
}

for x in `ls .`
do
    if [ -d "$x" ]; then
        echo "Running $x"
        pushd $x > /dev/null
        if [ -f "./run.sh" ]; then
            #Test has a specific run configuration; use it.
            ./run.sh
            record_pass $x $?
        elif [ -f "./Makefile" ]; then
            #Test involves weldr jobs.
            make clean all
            PRJ_PATH=`pwd`
            if [ -f "./test.sys" ]; then
                #Test has a pre-determined system configuration.
                SYS_PATH="$PRJ_PATH/test.sys"
                run_weldr $PRJ_PATH $x $SYS_PATH
                record_pass $x $? 
            else
                #Test is a number of isolated 
                SYS_PATH="$PRJ_PATH/tmp.sys"
                for y in `ls $PRJ_PATH`
                do
                    TST=$(echo "$y" | grep "\.exe")
                    if [ "$TST" != "" ] && [ -f $TST ]; then
                        echo $TST > $SYS_PATH
                        run_weldr $PRJ_PATH $y $SYS_PATH
                        record_pass $y $?
                    fi
                done
            fi
        fi
        popd > /dev/null
    fi
done
echo ""
echo "Passed $PASSED out of $TOTAL"
