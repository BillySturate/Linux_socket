/*************************************************************************
#	> File Name:test.cpp
#	> Author: Jay
#	> Mail: billysturate@gmail.com
#	> Created Time: Sun 06 Nov 2022 09:26:12 PM CST
 ************************************************************************/

#include <stdio.h>
int func(int src)
{
    return src+3.245;
}


int main()
{
    int a = 2;
    int b = func(a);
    printf("%d\n",b);
    return 0;
}


