#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
    int sockfd, connfd;


    //1.创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //2.初始化服务器地址
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9876);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //3.绑定套接字和地址
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    //4.把套接字转换成监听套接字
    listen(sockfd, 5);
    //5.接受连接请求
    struct sockaddr_in cliaddr;
    socklen_t cliaddr_len = sizeof(cliaddr);
    connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
    //6.收发数据
    char buf[1024];
    while (1)
    {
        bzero(buf, sizeof(buf));
        read(connfd, buf, sizeof(buf));
        printf("Received: %s\n", buf);
        write(connfd, buf, strlen(buf));
    }
    //7.关闭连接
    close(connfd);

    return 0;
}