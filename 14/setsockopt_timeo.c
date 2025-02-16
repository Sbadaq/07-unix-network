

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
//
//缺点：并非所有系统都支持，且不适用于 connect 函数。
int main(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    inet_pton(AF_INET,"127.0.0.1",&servaddr.sin_addr);

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
    setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv));

    close(sockfd);
    return 0;
}