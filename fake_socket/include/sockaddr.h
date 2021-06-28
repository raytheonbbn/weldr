// Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include <netinet/ip.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>

#define INET_LOOPBACK 0x0100007F
#define INET6_LOOPBACK_MSB 0x0
#define INET6_LOOPBACK_LSB 0x1

int compare_sockaddrs(const struct sockaddr* addr_a, const struct sockaddr* addr_b);
