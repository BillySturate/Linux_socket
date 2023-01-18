/*************************************************************************
#	> File Name:event.c
#	> Author: Jay
#	> Mail: billysturate@gmail.com
#	> Created Time: Fri 04 Nov 2022 04:23:33 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <event2/event.h>

int main(int argc, char *argv[])
{
    int i;
    struct event_base *base = event_base_new();
    const char **buf;
    const char *buf2;


    // buf = event_get_supported_methods();
    // for (i = 0; i < 10; i++) {
    //     printf("buf[i] = %s\n", buf[i]);
    // }

    buf2 = event_base_get_method(base);
    printf("%s\n", buf2);
    return 0;
}
