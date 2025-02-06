#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>

int main() {
    struct addrinfo hints, *res;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // IPv4 或 IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP 套接字

    status = getaddrinfo("www.deepseek.com", "http", &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    // 使用 res 中的地址信息
    // ...

    freeaddrinfo(res); // 释放 addrinfo 链表
    return 0;
}
