

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define MAXLINE 80
#define SERV_PORT 8000
int main(int argc, char *argv[]) {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    char buf[MAXLINE];
    time_t ticks;

     
    // 第一步：创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // 第二步：初始化服务器地址
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //第三步： 绑定套接字和地址（被动打开）
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    //第四步：把套接字转换成监听套接字，准备接受连接请求
    listen(sockfd, 5);
    printf("Server is running...\n");
    while (1) {
        cliaddr_len = sizeof(cliaddr);
        //第五步： 接受连接请求
        connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
        printf("Received a connection from %s\n", inet_ntoa(cliaddr.sin_addr));
        //第六步： 收发数据
        ticks = time(NULL);
        snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
        write(connfd, buf, strlen(buf));
        //第7步： 关闭连接
        close(connfd);
    }
    return 0;
}