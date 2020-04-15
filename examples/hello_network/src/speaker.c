// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "speaker.h"

int main(int argc, char *argv[]) {
    int sockfd = 0;
    int size = 0;
    char sendbuf[1024];
    char recvbuf[1024];
    struct sockaddr_in addr;

    memset(recvbuf, 0, 1024);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to create a socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
	addr.sin_port = htons(5000);

	if(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
		perror("Failed to resolve address");
		return 1;
	}

	if( connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Connect Failed");
		return 1;
    }

    printf("Connected to server. Please enter a password:\n>");

    fgets(sendbuf, 1023, stdin);
    strtok(sendbuf, "\n");

    printf("Read %s from stdin\n");

    write(sockfd, sendbuf, strlen(sendbuf));
    printf("Wrote sendbuf to socket\n");

    size = read(sockfd, recvbuf, 1023);
    if(size < 0) {
        perror("Read error");

        return 1;
    }
    recvbuf[size] = '\0';


    if(!strcmp(recvbuf, "success")) {
        printf("Yahoo!\n");
    } else {
        printf("Awwww :c\n");
    }

    return 0;
}
