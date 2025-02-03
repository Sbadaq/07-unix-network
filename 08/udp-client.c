
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
void dlg_cli(FILE* fp, int sockfd, struct sockaddr *servaddr, socklen_t len)
{
    int n;
    char buf[1024];
    while (fgets(buf,sizeof(buf),fp) != NULL)
    {
        sendto(sockfd, buf, strlen(buf), 0, servaddr, len);
        n = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);
        buf[n] = 0;
        printf("Received: %s", buf);
    }
}

int main()
{
    int sockfd;
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(1234);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    dlg_cli(stdin,sockfd,(struct sockaddr*)&servaddr, sizeof(servaddr));
}