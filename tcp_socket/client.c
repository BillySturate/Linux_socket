/*************************************************************************
#	> File Name:client.c
#	> Author: Jay
#	> Mail: billysturate@gmail.com
#	> Created Time: Sun 11 Sep 2022 07:58:17 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>

#define SERV_PORT 9527
void sys_err(const char *str)
{
    perror(str);
    exit(1);
}
int main(int argc, char *argv[])
{
    int cfd, ret;
    char buf[BUFSIZ];
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    ret = inet_pton(AF_INET, "10.0.12.16", (void *)&serv_addr.sin_addr.s_addr);
    if(ret == -1)
    {
        sys_err("tansform error");
    }
    cfd = socket(AF_INET, SOCK_STREAM, 0);
    if(cfd == -1)
    {
        sys_err("create error");
    }
    ret = connect(cfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(ret == -1)
    {
        sys_err("connect error");
    }
    while (1)
    {
        write(cfd, "hello\n", 6);
        ret = read(cfd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, ret);
        sleep(2);
    }
    

    return 0;
}
