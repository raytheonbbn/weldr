// Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "hello.h"

int main(int argc, char *argv[]) {
    std::string mystring;
    std::cin >> mystring;
    printf("C backup: %s\n", mystring.c_str());
    std::cout << "You gave me: ";
    std::cout << mystring;
    std::cout << std::endl;
    return 0;   
}
