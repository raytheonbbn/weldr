// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
//
// This document does not contain technology or Technical Data controlled under either
// the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
//
// Distribution A: Approved for Public Release, Distribution Unlimited
#include "broker.h"

char names[16][9];
char addrs[16][16];
int ports[16];

int next_user = 0;


int process_new_user(int sockfd) {
    char name[9];
    char addr[16];
    int port;
    printf("Received a request for a new user.\n");
    read(sockfd, name, 9);
    read(sockfd, addr, 16);
    read(sockfd, &port, 4);
    printf("Received sufficient data.  Working on it.\n");

    name[8] = 0;
    addr[15] = 0;

    int out;
    if(next_user >= 16) {
        out = REP_FAIL;
        write(sockfd, &out, 1); 
    } else {
        printf("Adding new user %s at %s:%d\n", name, addr, port);
        out = REP_OK;
        memcpy(names[next_user], name, 9);
        memcpy(addrs[next_user], addr, 16);
        ports[next_user] = port;
        next_user++;
        write(sockfd, &out, 1);
    }

    return 0;
}

int process_list(int sockfd) {
    int out = REP_OK;
    printf("Received a request for the user list.\n");
    write(sockfd, &out, 1);
    write(sockfd, &next_user, 4);
    for(int i = 0; i < next_user; i++) {
        write(sockfd, names[i], 9);
    }
    
    return 0;
}

int process_connect(int sockfd) {
    char name[9];
    read(sockfd, name, 9);
    int out;

    for(int i = 0; i < next_user; i++) {
        if(!strcmp(names[i], name)) {
            out = REP_OK;
            write(sockfd, &out, 1);
            write(sockfd, addrs[i], 16);
            return 0;
        }
    }
    out = REP_FAIL;
    write(sockfd, &out, 1);
    return 0;

}

int process_request(int sockfd) {
    int msg_type = 0;
    if(read(sockfd, &msg_type, 1) < 0) {
        perror("Read failed.");
        close(sockfd);
        return 1;
    }

    char name[8];

    switch(msg_type) {
        case MSG_CLOSE:
            close(sockfd);
            return 1;
        case MSG_REGISTER:
            return process_new_user(sockfd);
        case MSG_LIST:
            return process_list(sockfd);
        case MSG_CONN:
            return process_connect(sockfd);
        default:
            return 0;
    }
}


int main(int argc, char* argv[]) {
    char *addr = "127.0.0.1";
    int port = 8080;

    if(argc > 1) {
        port = atoi(argv[1]);
    }

    int server_sock = open_inbound_socket(port);
    if(server_sock < 0) {
        perror("Bad socket");
        exit(EXIT_FAILURE);
    }

    fd_set active_fd_set;
    fd_set read_fd_set;

    FD_ZERO(&active_fd_set);
    FD_SET(server_sock, &active_fd_set);

    while(1) {
        read_fd_set = active_fd_set;
        for(int i = 0; i < FD_SETSIZE; i++) {
            if(FD_ISSET(i, &read_fd_set)) {
                printf("Selecting on %d\n", i);
            }
        }
        if(select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            perror("Select failed.");
            exit(EXIT_FAILURE);
        }
        
        for(int i = 0; i < FD_SETSIZE; i++) {
            if(FD_ISSET(i, &read_fd_set)) {
                if(i == server_sock) {
                    int new_sock = accept_inbound_connection(server_sock);
                    if(new_sock < 0) {
                        printf("New socket is bad? %d\n", new_sock);
                        perror("Accept failed");
                        exit(EXIT_FAILURE);
                    }
                    FD_SET(new_sock, &active_fd_set);
                } else {
                    if(process_request(i)) {
                        FD_CLR(i, &active_fd_set);
                    }
                }
            }
        }

    }

    return 0;
}
