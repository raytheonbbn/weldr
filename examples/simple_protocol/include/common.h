// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


//Message types
#define  MSG_CLOSE 0
#define MSG_REGISTER 1
#define MSG_LIST 2
#define MSG_CONN 3

//Reply types
#define REP_OK 0
#define REP_FAIL 1

int open_inbound_socket(int port);
int accept_inbound_connection(int sockfd);
int open_outbound_socket(char* ip, int port);

#endif
