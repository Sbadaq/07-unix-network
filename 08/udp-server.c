

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

void dlg_echo(int sockfd,struct sockaddr *client_addr,int len)
{
    int n;
    char sendline[80];
    socklen_t cliaddr_len;
    while(1)
    {
        cliaddr_len = len;
        n = recvfrom(sockfd,sendline,80,0,client_addr,&cliaddr_len);
        printf("recvfrom message: %s\n",sendline);
        sendto(sockfd,sendline,n,0,client_addr,len);

    }
}

int main()
{
    int sockfd;
    struct sockaddr_in servaddr,cliaddr;
    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(1234);
    servaddr.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");
    //notes: bind is necessary for UDP server
    bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
    //notes:sockfd is for server,cliaddr is for client
    dlg_echo(sockfd,(struct sockaddr *)&cliaddr,sizeof(servaddr));
}