/**
 * 
 * @param 使用poll实现简单的udp服务器
 * 第一步 创建套接字并绑定地址
 * 第二步 初始化poolfd结构体数组，将udp套接字加入其中 并设置监听事件
 * 第三步 调用poll函数等待事件发生
 * 第四步 如果udp套接字可读，则接收数据并回复
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main()
{
    //1.创建套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    //2.初始化服务器地址
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9876);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //3.绑定套接字和地址
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    //4.初始化pollfd结构体数组
    struct pollfd fds[1];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    //5.调用poll函数等待事件发生
    int ret = poll(fds, 1, -1);
    //6.如果udp套接字可读，则接收数据并回复
    if (ret > 0)
    {
        if (fds[0].revents & POLLIN)
        {
            char buf[1024];
            struct sockaddr_in cliaddr;
            socklen_t cliaddr_len = sizeof(cliaddr);
            int n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &cliaddr_len);
            if (n > 0)
            {
                printf("Received: %s\n", buf);
                sendto(sockfd, buf, n, 0, (struct sockaddr *)&cliaddr, cliaddr_len);
            }
        }
    }
    //7.关闭套接字
    close(sockfd);
    return 0;
}