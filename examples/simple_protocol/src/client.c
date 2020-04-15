// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "client.h"

char *addr = "127.0.0.1";
int my_port = 0; 
int b_port = 0;

int start_chat(const char* name, const char* addr, const int port) {
    printf("Connecting to %s\n", name, addr, port);
    int sockfd = open_inbound_socket(my_port);
    printf("Please wait for recipient to respond...");
    int connfd = accept_inbound_connection(sockfd);

    if(connfd < 0) {
        return -1;
    }



    close(connfd);
    close(sockfd);
}

int process_register(int sockfd) {
    char name[9];
    int res;
    printf("Username? > ");
    fgets(name, 9, stdin);
    strtok(name, "\n");
    name[8] = 0;

    printf("Thank you.  Registering '%s'\n", name);
    res = MSG_REGISTER;
    write(sockfd, &res, 1);
    write(sockfd, name, 9);
    write(sockfd, addr, 16);
    write(sockfd, &my_port, 4);
    printf("Wrote data.  waiting for reply\n");

    read(sockfd, &res, 1);

    if(res == REP_OK) {
        printf("Ok\n");
    } else {
        printf("Failure\n");
    }

    return 0;
}

int process_list(int sockfd) {
    int res;
    int count;
    res = MSG_LIST;
    write(sockfd, &res, 1);

    read(sockfd, &res, 1);
    if(res == REP_OK) {
        printf("Ok\n");
        read(sockfd, &count, 4);
        for(int i = 0; i < count; i++) {
            char name[9];
            read(sockfd, name, 9);
            printf("User: %s\n", name);
        }
    } else {
        printf("Failure\n");
    }
    return 0;
}

int process_connect(int sockfd) {
    int res;
    char name[9];
    char addr[16];
    int port;

    printf("Recipient? > ");
    fgets(name, 9, stdin);
    strtok(name, "\n");
    name[8] = 0;

    res = MSG_CONN;
    write(sockfd, &res, 1);
    write(sockfd, name, 9);
    read(sockfd, &res, 1);
    if(res == REP_OK) {
        read(sockfd, addr, 16);
        read(sockfd, &port, 4);
        return start_chat(name, addr, port); 
    } else {
        printf("Unknown recipient\n");
    }
}

int process_check(int sockfd) {
    return 0;
}

int process_accept(int sockfd) {
    return 0;
}


int process_cmd(int sockfd, char *cmd) {
    if(!strcmp(cmd, CMD_QUIT)) {
        return 1;
        printf("Goodbye\n");
    } else if(!strcmp(cmd, CMD_REGISTER)) {
        return process_register(sockfd);
    } else if(!strcmp(cmd, CMD_LIST)) {
        return process_list(sockfd);
    } else if(!strcmp(cmd, CMD_CONN)) {
        return process_connect(sockfd);
    } else if(!strcmp(cmd, CMD_CHECK)) {
        return process_check(sockfd);
    } else if(!strcmp(cmd, CMD_ACCEPT)) {
        return process_accept(sockfd);
    } else {
        return 0;
    }

}

int main(int argc, char* argv[]) {


    if(argc > 2) {
        my_port = atoi(argv[1]);
        b_port = atoi(argv[2]);
    } else {
        printf("Usage: client local_port, broker_port");
    }

    int sockfd = open_outbound_socket(addr, b_port);
    if(sockfd < 0) {
        perror("Connect failed.");
        exit(EXIT_FAILURE);
    }

    char cmd[16];
    while(1) {
        printf("> ");
        fgets(cmd, 16, stdin);
        strtok(cmd, "\n");
        printf("Executing '%s'\n", cmd);
        if(process_cmd(sockfd, cmd)) {
            close(sockfd);
            return 0;
        }
    }
}
