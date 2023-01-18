/*************************************************************************
#	> File Name:mywrite.c
#	> Author: Jay
#	> Mail: billysturate@gmail.com
#	> Created Time: Fri 04 Nov 2022 06:29:37 PM CST
 ************************************************************************/

/*************************************************************************
#	> File Name:myread.c
#	> Author: Jay
#	> Mail: billysturate@gmail.com
#	> Created Time: Fri 04 Nov 2022 06:29:29 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <event2/event.h>
#include <sys/stat.h>
#include <fcntl.h>

void sys_err(const char *str)
{
    perror(str);
    exit(1);
}

void write_cb(evutil_socket_t fd, short what, void *arg)
{
    char buf[] = "hello libevent";
    write(fd, buf, strlen(buf) + 1);
    // printf("what = %s , read from write: %s\n", what & EV_READ ? "read满足" : "read不满足", buf);
    sleep(1);

    return;
}

int main(int argc, char *argv[])
{
    //打开FIFO的写端
    int fd = open("testfifo", O_WRONLY | O_NONBLOCK);
    if (fd == -1)
    {
        sys_err("open error");
    }

    //创建event_base
    struct event_base *base = event_base_new();

    //创建事件
    struct event *ev = NULL;
    ev = event_new(base, fd, EV_WRITE | EV_PERSIST, write_cb, NULL);

    //添加事件到event_base上
    event_add(ev, NULL);

    //启动循环
    event_base_dispatch(base);

    //销毁event_base
    event_base_free(base);
    return 0;
}
