/*************************************************************************
#	> File Name:client.c
#	> Author: Jay
#	> Mail: billysturate@gmail.com
#	> Created Time: Tue 08 Nov 2022 03:10:51 PM CST
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAXLINE 80
#define SERV_PORT 9527
int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr;
	char buf[MAXLINE];
	int sockfd, n;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("create failed");
        exit(1);
    }

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, "124.221.165.184", &servaddr.sin_addr);
	servaddr.sin_port = htons(SERV_PORT);

	int i = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (i < 0)
    {
        perror("connect failed");
        exit(1);
    }
    int num = 0;
    printf("服务器连接成功\n");
    while (1)
    {
        sprintf(buf, "hello, world, %d\n...", num++);
        printf("%s\n", buf);
        write(sockfd, buf, strlen(buf) + 1);
        recv(sockfd, buf, sizeof(buf), 0);
        printf("recv msg:%s\n", buf);
        usleep(10000);
    }
    recv(sockfd, buf, sizeof(buf), 0);
    printf("recv msg:%s\n", buf);	
    printf("over-----------\n");
	close(sockfd);
	return 0;
}

