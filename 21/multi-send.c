#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sockfd;
    struct sockaddr_in multicast_addr;
    const char *message = "Hello, Multicast!";

    // 创建 UDP 套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // 设置多播地址
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_port = htons(12345);
    inet_pton(AF_INET, "239.255.255.250", &multicast_addr.sin_addr);

    // 发送多播消息
    if (sendto(sockfd, message, strlen(message), 0,
               (struct sockaddr *)&multicast_addr, sizeof(multicast_addr)) < 0) {
        perror("sendto");
        close(sockfd);
        exit(1);
    }

    printf("Multicast message sent: %s\n", message);
    close(sockfd);
    return 0;
}