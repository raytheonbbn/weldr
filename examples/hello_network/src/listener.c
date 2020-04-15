// Copyright (c) 2019, Raytheon BBN Technologies, Inc. All Rights Reserved.
// This document does not contain technology or Technical Data controlled under either
// the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#include "listener.h"

int main(int argc, char *argv[])
{
	int listenfd = 0;
    int connfd = 0;
    int size = 0;
	struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

    const char *secret = "foobar";
    const char *success = "success";
    const char *failure = "failure";

	char sendbuf[1024];
    char recvbuf[1024];

	/* creates an UN-named socket inside the kernel and returns
	 * an integer known as socket descriptor
	 * This function takes domain/family as its first argument.
	 * For Internet family of IPv4 addresses we use AF_INET
	 */
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(&client_addr, '0', sizeof(client_addr));
	memset(sendbuf, '0', sizeof(sendbuf));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(5000);


    //fprintf(stderr, "sin_addr: %lu\n", serv_addr.sin_addr.s_addr);
    //char *test = inet_ntoa(serv_addr.sin_addr);

	/* The call to the function "bind()" assigns the details specified
	 * in the structure 『serv_addr' to the socket created in the step above
	 */
	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 10);

	for(int i = 0; i < 1; i++);
	{
		/* In the call to accept(), the server is put to sleep and when for an incoming
		 * client request, the three way TCP handshake* is complete, the function accept()
		 * wakes up and returns the socket descriptor representing the client socket.
		 */
        int size = sizeof(client_addr);
		connfd = accept(listenfd, (struct sockaddr*)&client_addr, &size);

        printf("We got a connection!\n");

        size = read(connfd, recvbuf, 1023);
        if(size < 0) {
            perror("Read error");
            return 1;
        }
        recvbuf[size] = '\0';
        printf("We got a message\n");


		if(!strcmp(secret, recvbuf)) {
            write(connfd, success, strlen(success));
        } else {
            write(connfd, failure, strlen(failure));
        }

		close(connfd);
		sleep(1);
	}
}
