/*************************************************************************
#	> File Name:myhttp.c
#	> Author: Jay
#	> Mail: billysturate@gmail.com
#	> Created Time: Sat 07 Jan 2023 05:21:06 PM CST
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>

#define MAXSIZE 9527

void send_error(int cfd, int status, char *title, char *text)
{
	char buf[4096] = {0};
    printf("status = %d, title = %s\n", status, title);

	sprintf(buf, "%s %d %s\r\n", "HTTP/1.1", status, title);
	sprintf(buf+strlen(buf), "Content-Type: %s\r\n", "text/html; charset=utf-8");
	sprintf(buf+strlen(buf), "Content-Length: %d\r\n", -1);
	sprintf(buf+strlen(buf), "Connection: close\r\n");
    printf("buf = %s\n", buf);
	int ret = send(cfd, buf, strlen(buf), 0);
    printf("ret = %d\n", ret);
	send(cfd, "\r\n", 2, 0);

	memset(buf, 0, sizeof(buf));

	// sprintf(buf, "<html><head><title>%d %s</title></head>\n", status, title);
	// sprintf(buf+strlen(buf), "<body bgcolor=\"#cc99cc\"><h2 align=\"center\">%d %s</h2>\n", status, title);
	// sprintf(buf+strlen(buf), "%s\n", text);
	// sprintf(buf+strlen(buf), "<hr>\n</body>\n</html>\n");
    // sprintf(buf, "<html><head><title>%d %s</title></head>", status, title);
    strcpy(buf, "<html><head><title>Document</title></head><body><h1>一级标题</h1><h2>二级标题</h2></body></html>");
    printf("buf = %s\n", buf);
	ret = send(cfd, buf, strlen(buf), 0);
    printf("ret = %d\n", ret);
	
	return ;
}

// 获取一行 \r\n 结尾的数据 
int get_line(int cfd, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;
    while ((i < size-1) && (c != '\n')) {  
        n = recv(cfd, &c, 1, 0);
        if (n > 0) {     
            if (c == '\r') {            
                n = recv(cfd, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n')) {              
                    recv(cfd, &c, 1, 0);
                } else {                       
                    c = '\n';
                }
            }
            buf[i] = c;
            i++;
        } else {      
            c = '\n';
        }
    }
    buf[i] = '\0';
    
    if (-1 == n)
    	i = n;

    return i;
}

int init_listen_fd(int port, int epfd)
{
    //　创建监听的套接字 lfd
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {    
        perror("socket error");
        exit(1);
    }
    // 创建服务器地址结构 IP+port
    struct sockaddr_in srv_addr;
    
    bzero(&srv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 端口复用
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 给 lfd 绑定地址结构
    int ret = bind(lfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if (ret == -1) {   
        perror("bind error");
        exit(1);
    }
    // 设置监听上限
    ret = listen(lfd, 128);
    if (ret == -1) { 
        perror("listen error");
        exit(1);
    }
    
    // lfd 添加到 epoll 树上
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if (ret == -1) { 
        perror("epoll_ctl add lfd error");
        exit(1);
    }

    return lfd;
}

void do_accept(int lfd, int epfd)
{
	struct sockaddr_in clt_addr;
    socklen_t clt_addr_len = sizeof(clt_addr);
    
    int cfd = accept(lfd, (struct sockaddr*)&clt_addr, &clt_addr_len);
    if (cfd == -1) {   
        perror("accept error");
        exit(1);
    }

    // 打印客户端IP+port
    char client_ip[64] = {0};
    printf("New Client IP: %s, Port: %d, cfd = %d\n",
           inet_ntop(AF_INET, &clt_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
           ntohs(clt_addr.sin_port), cfd);

    // 设置 cfd 非阻塞
    int flag = fcntl(cfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(cfd, F_SETFL, flag);

    // 将新节点cfd 挂到 epoll 监听树上
    struct epoll_event ev;
    ev.data.fd = cfd;
    
    // 边沿非阻塞模式
    ev.events = EPOLLIN | EPOLLET;
    
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    if (ret == -1)  {
        perror("epoll_ctl add cfd error");
        exit(1);
    }
}

// 断开连接
void disconnect(int cfd, int epfd)
{
    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
    if (ret != 0)
    {
        perror("epoll ctl error");
        exit(1);
    }
    close(cfd);
}

//客户端的fd，错误号，错误描述，回发文件类型，文件长度
void send_respond(int cfd, int no, const char *disp, const char *type, int len)
{
    char buf[4096] = {0};

	sprintf(buf, "HTTP/1.1 %d %s\r\n", no, disp);
	sprintf(buf + strlen(buf), "Content-Type:%s\r\n", type);
	sprintf(buf + strlen(buf), "Content-Length:%d\r\n", len);

	send(cfd, buf, strlen(buf), 0);
	send(cfd, "\r\n", 2, 0);
}

//发送服务器本地文件给浏览器
void send_file(int cfd, const char *file)
{
    int n = 0, ret = 0;
    char buf[4096] = {0};
    int fd = open(file, O_RDONLY);
    if (fd == -1)
    {
        perror("open error!");
        send_error(cfd, 404, "Not Found", "NO such file or direntry");
        exit(1);
        // return;
    }
    while ((n = read(fd, buf, sizeof(buf))) > 0)
    {
        printf("n=%d\n", n);
        ret = write(cfd, buf, n);
        // ret = send(cfd, buf, n, 0);       
        usleep(100);
        // sleep(1);
        printf("ret=%d\n", ret);
        if (ret == -1)
        {
            printf("errno = %d\n", errno);
            if (errno == EAGAIN)
            {
                perror("send error");
                // printf("--------EAGAIN!!\n");
                continue;
            } else if (errno == EINTR) {
                // printf("--------EINTR!!\n");
                perror("send error");
                continue;
            // } else if (errno == ECONNRESET) {
            //     perror("send error:");
            //     break;
            } else {
                perror("send error");
                exit(1);
            }          
        }
    }
        if(n == -1)  {  
            perror("read file error");
            exit(1);
        }
    close(fd);
}


// void send_file(int cfd, const char *file)
// {
//     int fd = open(file, O_RDONLY);
//     assert(fd > 0);
// #if 1
//     while (1)
//     {
//         char buf[1024] = {0};
//         int len = read(fd, buf, sizeof(buf));
//         // printf("len = %d\n", len);
//         if ( len > 0)
//         {
//             int n = send(cfd, buf, len, 0);
//             printf("n = %d\n", n);
//             usleep(10);  //这很重要
//         } else if (len == 0) {
//             break;
//         }  else {
//             perror("read error");
//         }
//     }
// #else
//     off_t offset = 0;
//     int size = lseek(fd, 0, SEEK_END);
//     lseek(fd, 0, SEEK_SET);
//     while (offset < size)
//     {
//         int ret = sendfile(cfd, fd, &offset, size);
//         printf("ret value: %d\n", ret);
//         if (ret == -1 && errno == EAGAIN)
//         {
//             printf("没数据---\n");
//         }        
//     }  
// #endif
//     close(fd);
// }



// 通过文件名获取文件的类型
const char *get_file_type(const char *name)
{
    char* dot;

    // 自右向左查找‘.’字符, 如不存在返回NULL
    dot = strrchr(name, '.');   
    if (dot == NULL)
        return "text/plain; charset=utf-8";
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp( dot, ".wav" ) == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}


//处理http请求，判断文件是否存在，回发
void http_request(int cfd, const char *file)
{
    struct stat sbuf;
    //判断文件是否存在
    int ret = stat(file, &sbuf);
    if (ret != 0)
    {
        //回发服务器404错误页面
        perror("stat error!");
        send_error(cfd, 404, "Not Found", "NO such file or direntry");
        return;
        // exit(1);
    }
    if (S_ISREG(sbuf.st_mode))
    {
        printf("---it's a file\n");
        //回发http协议应答
        // send_respond(cfd, 200, "OK", "Content-Type: image/jpeg", sbuf.st_size);
        // send_respond(cfd, 200, "OK", "Content-Type: audio/mpeg", sbuf.st_size);
        send_respond(cfd, 200, "OK", get_file_type(file), sbuf.st_size);
        //回发给客户端请求数据内容
        send_file(cfd, file);
    }
    
}

void do_read(int cfd, int epfd)
{
	// read cfd 小 -- 大 write 回
	// 读取一行http协议， 拆分， 获取 get 文件名 协议号
    char line[1024] = {0};
    int len = get_line(cfd, line, sizeof(line));     //读http请求协议首行 GET/hello.c HTTP/1.1
    if (len == 0)
    {
        printf("服务器，检查到客户端关闭....\n");
        disconnect(cfd, epfd);
    }else{
        char method[16], path[256], protocal[16];
        sscanf(line, "%[^ ] %[^ ] %[^ ]", method, path, protocal);
        printf("method = %s, path = %s, protocal = %s\n", method, path, protocal);
        while (1)
        {
            char buf[1024] = {0};
            len = get_line(cfd, buf, sizeof(buf));
            if (len == -1)
            {
                break;
            }
            // printf("len = %d --- %s", len, buf);           
        }
        if (strncasecmp(method, "GET", 3) == 0)
        {
            char *file = path + 1;
            http_request(cfd, file);
        }
        
    }
    
}

void epoll_run(int port)
{
	int i = 0;
    struct epoll_event all_events[MAXSIZE];

    // 创建一个epoll监听树根
    int epfd = epoll_create(MAXSIZE);
    if (epfd == -1) { 
        perror("epoll_create error");
        exit(1);
    }
    
    // 创建lfd，并添加至监听树
    int lfd = init_listen_fd(port, epfd);
   
    while (1) {
    	// 监听节点对应事件
        int ret = epoll_wait(epfd, all_events, MAXSIZE, -1);
        if (ret == -1) {      
            perror("epoll_wait error");
            exit(1);
        }

        for (i=0; i<ret; ++i) {
        	    
            // 只处理读事件, 其他事件默认不处理
            struct epoll_event *pev = &all_events[i];
            
            // 不是读事件
            if (!(pev->events & EPOLLIN)) {                     
                continue;
            }
            if (pev->data.fd == lfd) {   	// 接受连接请求   
                
                do_accept(lfd, epfd);
                
            } else {						// 读数据
                
                do_read(pev->data.fd, epfd);
            }
        }
    }
}


int main(int argc, char *argv[])
{ 
    // 命令行参数获取 端口 和 server提供的目录
    if (argc < 3) 
    {
    	printf("./server port path\n");	
    }
    
    // 获取用户输入的端口 
    int port = atoi(argv[1]);
    
    // 改变进程工作目录
    int ret = chdir(argv[2]);
    if (ret != 0) {
    	perror("chdir error");	
    	exit(1);
    }

	// 启动 epoll监听
	epoll_run(port);

    return 0;
}



