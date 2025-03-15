#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/sctp.h>

int main() {
    int sockfd, connfd;
    struct sockaddr_in server_addr;
    char buffer[1024];

    // 创建 SCTP 套接字
    sockfd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // 绑定到本地地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    // 监听连接
    listen(sockfd, 5);

    // 接受连接
    connfd = accept(sockfd, NULL, NULL);
    if (connfd < 0) {
        perror("accept");
        close(sockfd);
        exit(1);
    }

    // 接收数据
    ssize_t len = sctp_recvmsg(connfd, buffer, sizeof(buffer), NULL, 0, NULL, 0);
    if (len < 0) {
        perror("sctp_recvmsg");
    } else {
        buffer[len] = '\0';
        printf("Received: %s\n", buffer);
    }

    close(connfd);
    close(sockfd);
    return 0;
}