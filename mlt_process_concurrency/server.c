/*************************************************************************
#	> File Name:server.c
#	> Author: Jay
#	> Mail: billysturate@gmail.com
#	> Created Time: Tue 13 Sep 2022 07:21:20 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "wrap.h"

#define SRV_PORT 9527

void catch_child(int argc)
{
    while (waitpid(0, NULL, WNOHANG) > 0)
    {
        
    }
    return;

}
int main(int argc, char *argv[])
{
    int lfd,cfd;
    pid_t pid;
    char buf[BUFSIZ];
    struct sockaddr_in srv_addr, clit_addr;
    socklen_t clt_addr_len;
    int ret, i;

    bzero(&srv_addr, sizeof(srv_addr));
    // memset(&srv_addr, 0, sizeof(srv_addr));

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(SRV_PORT);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    lfd = Socket(AF_INET, SOCK_STREAM, 0);

    Bind(lfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

    Listen(lfd, 128);

    clt_addr_len = sizeof(clit_addr);

    while (1)
    {
        cfd = Accept(lfd, (struct sockaddr *)&clit_addr, &clt_addr_len);
        pid = fork();
        if (pid < 0)
        {
            perr_exit("fork error");
        }else if(pid == 0){
            close(lfd);
            break;
        }else{
            struct sigaction act;
            act.sa_handler = catch_child;
            sigemptyset(&act.sa_mask);
            act.sa_flags = 0;
            ret = sigaction(SIGCHLD, &act, NULL);
            if(ret == -1){
                perr_exit("sigaction error");
            }
            close(cfd);
            continue;
        }
    }

        if(pid == 0)
        {
            while(1){
                ret = Read(cfd, buf, sizeof(buf));
                if(ret == 0)
                {
                    close(cfd);
                    exit(1);
                }
                for(i = 0; i < ret; i++)
                {
                    buf[i] = toupper(buf[i]);
                }
                write(cfd, buf, ret);
                write(STDOUT_FILENO, buf, ret);
            }  
        }
    return 0;
}
