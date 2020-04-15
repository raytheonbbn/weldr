#!/bin/bash
#Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
type -p dnf >&/dev/null
if [ $? -eq 0 ]; then
    dnf groups summary "C Development Tools and Libraries" | grep "Installed" &> /dev/null
    if [[ $? -ne 0 ]]; then
        sudo dnf -y group install "C Development Tools and Libraries"
        if [[ $? -ne 0 ]]; then
            echo "C developer tools install failed"
            exit 1
        fi
    fi
    exit 0
fi
type -p yum >&/dev/null
if [ $? -eq 0 ]; then
    yum groups list installed Development | grep -q "Development Tools"
    if [[ $? -ne 0 ]]; then
        sudo yum groupinstall -y development
        if [[ $? -ne 0 ]]; then
            echo "Development Tools Group Install Failed"
            exit 1
        fi
    fi
    exit 0
fi

