#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>


int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM,0);
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serveraddr.sin_addr);

    int flags = fcntl(sockfd, F_GETFL,0);//set socket to non-blocking
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    //由于套接字是非阻塞的，如果连接不能立即建立，connect()函数将返回-1，并将errno设置为EINPROGRESS，表示连接操作正在进行中。
    int connect_ret = connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if(connect_ret == -1 && errno == EINPROGRESS)
    {
        perror("connecting error\n");
        //因为EINPROGRESS是一个预期中的状态，而不是错误），然后程序可以选择执行其他操作（这里用注释表示）。
         
    }

    //使用select()函数等待套接字的写操作变为就绪（即连接完成或失败）。这里设置了一个5秒的超时时间。
    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sockfd, &writefds);//
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    int ret = select(sockfd+1, NULL, &writefds, NULL, &tv);
    /** 
        如果ret为-1，表示select()调用出错。
        如果ret为0，表示超时，连接未完成。
        如果ret大于0，表示有文件描述符就绪。
    */
    if(ret == -1)
    {
        perror("select error\n");
        return -1;
    }
    else if(ret == 0)
    {
        printf("connect timeout!\n");
        return -1;
    }
    else 
    {
        int error;
        socklen_t len = sizeof(error);
        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
        if(error)
        {
            perror("connect error\n");
            return -1;
        }
        else
        {
            printf("connect success\n");
            return 0;
        }
    }

    fcntl(sockfd,F_SETFL,flags);
    close(sockfd);

    return 0;
}