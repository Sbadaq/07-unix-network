
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    
    //1.创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //2. 初始化服务器地址
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9876);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    //3. 连接服务器
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    //4. 收发数据
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
    //5. 关闭连接
    close(sockfd);

    return 0;
}