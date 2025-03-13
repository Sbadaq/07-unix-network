
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <strings.h>
int main(int argc, char **argv)
{
    int sockfd;
    struct linger ling;
    struct sockaddr_in servaddr;

    if(argc != 2)
    {
        printf("usage: %s <IPaddress>\n", argv[0]);
        exit(1);

    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    int err = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(err < 0)
    {
        printf("connect error\n");
        exit(1);
    }
    ling.l_onoff = 1;
    ling.l_linger = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
    close(sockfd);
    return 0;
}