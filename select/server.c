/*************************************************************************
#	> File Name:server.c
#	> Author: Jay
#	> Mail: billysturate@gmail.com
#	> Created Time: Thu 06 Oct 2022 10:12:32 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "wrap.h"

#define SERV_PORT 9527

int main(int argc, char *argv[])
{
    int listenfd, connfd;
    char buf[BUFSIZ];

    struct sockaddr_in clie_addr, serv_addr;
    socklen_t clie_addr_len;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);
    Bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    Listen(listenfd, 128);

    fd_set rset, allset;        //定义读集合，备份集合
    int ret, maxfd = 0, n, i, j;
    maxfd = listenfd;       //最大文件描述符

    FD_ZERO(&allset);       //清空监听集合
    FD_SET(listenfd, &allset);      //将待监听fd添加到监听集合中

    while (1)
    {
        rset = allset;      //备份
        ret = select(maxfd+1, &rset, NULL, NULL, NULL);     //使用select监听
        if (ret < 0)
        {
            perr_exit("select error");
        }
        if (FD_ISSET(listenfd, &rset))      //listenfd满足监听的读事件
        {
            clie_addr_len = sizeof(clie_addr);
            connfd = Accept(listenfd, (struct sockaddr *)&clie_addr, &clie_addr_len);       //建立连接    -----不会阻塞
            FD_SET(connfd, &allset);        //将新产生的fd,添加到监听集合中,监听数据读事件

            if(maxfd < connfd)      //修改maxfd
            {
                maxfd = connfd;
            }

            if(ret == 1)        //说明select只返回一个,并且是listenfd,后续执行无需执行
            {
                continue;
            }
        }
        for(i = listenfd + 1; i <= maxfd; i++){     //处理满足读事件的fd
            if(FD_ISSET(i, &allset))        //找到满足读事件的那个fd
            {
                n = Read(i, buf, sizeof(buf));
                if(n == 0)      //检测到客户端已经关闭连接
                {
                    Close(i);
                    FD_CLR(i, &allset);     //将关闭的fd，移除出监听集合
                }else if(n == -1){
                    perr_exit("read error");
                }

                for(j =0; j< n; j++)
                {
                    buf[j] = toupper(buf[j]);
                }
                write(i, buf, n);
                write(STDOUT_FILENO, buf, n);

            }
        }     
    }
    Close(listenfd);

    return 0;
}
