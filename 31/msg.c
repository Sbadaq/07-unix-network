#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[1024];

    // 创建 UDP 套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // 发送数据
    const char *message = "Hello, UDP!";
    sendto(sockfd, message, strlen(message), 0,
           (struct sockaddr *)&server_addr, sizeof(server_addr));

    // 接收数据
    ssize_t len = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
    if (len < 0) {
        perror("recvfrom");
        close(sockfd);
        exit(1);
    }

    buffer[len] = '\0';
    printf("Received: %s\n", buffer);

    // 关闭套接字
    close(sockfd);
    return 0;
}