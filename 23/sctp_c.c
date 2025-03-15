#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/sctp.h>

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    const char *message = "Hello, SCTP!";

    // 创建 SCTP 套接字
    sockfd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // 连接服务器
    if (sctp_connectx(sockfd, (struct sockaddr *)&server_addr, 1, NULL) < 0) {
        perror("sctp_connectx");
        close(sockfd);
        exit(1);
    }

    // 发送数据
    if (sctp_sendmsg(sockfd, message, strlen(message), NULL, 0, 0, 0, 0, 0, 0) < 0) {
        perror("sctp_sendmsg");
        close(sockfd);
        exit(1);
    }

    printf("Message sent: %s\n", message);
    close(sockfd);
    return 0;
}