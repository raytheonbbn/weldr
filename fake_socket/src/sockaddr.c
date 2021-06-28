// Copyright (c) 2020 Raytheon BBN Technologies, Inc.  All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include "sockaddr.h"

void inet6_to_long(const struct sockaddr_in6* addr, uint64_t* msb, uint64_t* lsb) {
    uint64_t m_lsb = 0;
    uint64_t m_msb = 0;
    for(int i = 0; i < 8; i++) {
        m_msb = m_msb << 8;
        m_msb |= addr->sin6_addr.s6_addr[i];
    }
    for(int i = 8; i < 16; i++) {
        m_lsb = m_lsb << 8;
        m_lsb |= addr->sin6_addr.s6_addr[i];
    }
    *msb = m_msb;
    *lsb = m_lsb;
}

int compare_inet6_sockaddrs(const struct sockaddr_in6* addr_a, const struct sockaddr_in6* addr_b) {
    in_port_t port_a = addr_a->sin6_port;
    in_port_t port_b = addr_b->sin6_port;
    // Check that the ports are equal.
    if(port_a != port_b) {
        return 0;
    }

    uint64_t addr_a_msb = 0;
    uint64_t addr_a_lsb = 0;
    inet6_to_long(addr_a, &addr_a_msb, &addr_a_lsb);


    uint64_t addr_b_msb = 0;
    uint64_t addr_b_lsb = 0;
    inet6_to_long(addr_b, &addr_b_msb, &addr_b_lsb);

    // Check if the addresses are equal.
    if(addr_a_msb == addr_b_msb && addr_a_lsb == addr_a_lsb) {
        return 1;
    }

    // Check if one of the addresses matches the other.
    // FIXME: This is currently hacked to assume null and loopback  match everything.
    if(addr_a_msb == 0 && addr_a_lsb == 0) {
        return 1;
    }
    if(addr_a_msb == INET6_LOOPBACK_MSB && addr_a_lsb == INET6_LOOPBACK_LSB) {
        return 1;
    }
    if(addr_b_msb == 0 && addr_b_lsb == 0) {
        return 1;
    }
    if(addr_b_msb == INET6_LOOPBACK_MSB && addr_b_lsb == INET6_LOOPBACK_LSB) {
        return 1;
    }
    return 0;
}

int compare_inet_sockaddrs(const struct sockaddr_in* addr_a, const struct sockaddr_in* addr_b) {
    in_port_t port_a = addr_a->sin_port;
    in_port_t port_b = addr_b->sin_port;
    // Check that the ports are equal.
    if(port_a != port_b) {
        return 0;
    }

    uint32_t s_addr_a = addr_a->sin_addr.s_addr;
    uint32_t s_addr_b = addr_b->sin_addr.s_addr;
    // Check if the addresses are equal.
    if(s_addr_a == s_addr_b) {
        return 1;
    }
    // Check if one of the addresses matches the other.
    // FIXME: This is currently hacked to assume null and loopback match everything.
    if(s_addr_a == 0 || s_addr_a == INET_LOOPBACK) {
        return 1;
    }
    if(s_addr_b == 0 || s_addr_b == INET_LOOPBACK) {
        return 1;
    }
    return 0;
}

int compare_sockaddrs(const struct sockaddr* addr_a, const struct sockaddr* addr_b) {
    // Extract the family IDs.
    sa_family_t family_a = addr_a->sa_family;
    sa_family_t family_b = addr_b->sa_family;

    // Make sure the structs are compatible
    if(family_a != family_b) {
        fprintf(stderr, "ABORT: Tried comparing sockaddrs from different families: %d and %d\n", family_a, family_b);
        exit(1);
    }

    //Run the correct subroutine based on the family.
    switch(family_a) {
        case AF_INET:
            return compare_inet_sockaddrs((const struct sockaddr_in *)addr_a, (const struct sockaddr_in *)addr_b);
        case AF_INET6:
            return compare_inet6_sockaddrs((const struct sockaddr_in6 *)addr_a, (const struct sockaddr_in6 *)addr_b);
        default:
            fprintf(stderr, "ABORT: Unhandled family: %d\n", family_a);
            exit(1);
    }
}
