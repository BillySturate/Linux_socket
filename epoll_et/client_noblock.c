/*************************************************************************
#	> File Name:client_noblock.c
#	> Author: Jay
#	> Mail: billysturate@gmail.com
#	> Created Time: Sun 23 Oct 2022 03:13:29 PM CST
 ************************************************************************/

/* client.c */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "wrap.h"

#define MAXLINE 10
#define SERV_PORT 8080

int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr;
	char buf[MAXLINE];
	int sockfd, i;
	char ch = 'a';

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
	servaddr.sin_port = htons(SERV_PORT);

	connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	while (1) {
		for (i = 0; i < MAXLINE/2; i++)
			buf[i] = ch;
		buf[i-1] = '\n';
		ch++;

		for (; i < MAXLINE; i++)
			buf[i] = ch;
		buf[i-1] = '\n';
		ch++;

		write(sockfd, buf, sizeof(buf));
		sleep(10);
	}
	Close(sockfd);
	return 0;
}

