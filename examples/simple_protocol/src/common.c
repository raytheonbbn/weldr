// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "common.h"

int open_inbound_socket(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    fprintf(stdout, "Binding to %s:%hd\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    if(bind(sockfd, (struct sockaddr*) &addr, sizeof(addr))) {
        return -1;
    }

    if(listen(sockfd, 10)) {
        return -1;
    }

    fprintf(stdout, "Bound on FD %d\n", sockfd);
    return sockfd;
}

int accept_inbound_connection(int sockfd) {
    struct sockaddr_in addr;
    socklen_t len;
    int new_fd;
    if((new_fd = accept(sockfd, (struct sockaddr*) &addr, &len)) < 0) {
        return -1;
    } else {
        fprintf(stdout, "Connection received from %s:%hd on FD %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), new_fd);
        return new_fd;
    }


}

int open_outbound_socket(char* ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton(ip, &(addr.sin_addr));

    if(connect(sockfd, (struct sockaddr*) &addr, sizeof(struct sockaddr_in))) {
        return -1;
    }
    return sockfd;
}
