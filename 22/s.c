#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[1024];

    // 创建 UDP 套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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

    // 接收数据
    socklen_t addr_len = sizeof(client_addr);
    ssize_t len = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                           (struct sockaddr *)&client_addr, &addr_len);
    if (len < 0) {
        perror("recvfrom");
    } else {
        buffer[len] = '\0';
        printf("Received: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}