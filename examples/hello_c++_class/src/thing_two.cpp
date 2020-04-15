// Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "hello.h"
#include "thing_two.h"

class Thing {
public:
    std::string get_name() {
        return "bar";
    }
    std::string get_purpose() {
        return "joy";
    }
};

void thing_two_speak() {
    Thing thing;
    std::cout << thing.get_name();
    std::cout << thing.get_purpose();
}
