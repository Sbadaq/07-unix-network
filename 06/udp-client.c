
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main()
{
    //1.socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    //2.初始化服务器地址
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9876);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    int ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (ret < 0)
    {
        perror("connect");
        exit(1);
    }
    //3.收发数据
    char buf[1024];
    while (1)
    {
        bzero(buf, sizeof(buf));
        fgets(buf, sizeof(buf), stdin);
        write(sockfd, buf, strlen(buf));
        bzero(buf, sizeof(buf));
        int n = read(sockfd, buf, sizeof(buf));
        if (n == 0)
        {
            printf("Server closed\n");
            break;
        }
        printf("Received: %s\n", buf);
    }
    close(sockfd);
}