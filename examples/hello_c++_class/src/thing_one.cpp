// Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include "hello.h"
#include "thing_one.h"

class Thing {
public:
    std::string get_name() {
        return "foo";
    }
};

void thing_one_speak() {
    Thing thing;
    std::cout << thing.get_name();
}


