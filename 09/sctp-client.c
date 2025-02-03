
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/sctp.h>

int main()
{
     
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    int sockfd = socket(AF_INET,SOCK_SEQPACKET,IPPROTO_SCTP);
    connect(sockfd,(struct sockaddr *)&server,sizeof(server));
    while(1)
    {
        char buffer[1024];
        fgets(buffer,1024,stdin);
        send(sockfd,buffer,strlen(buffer),0);
        recv(sockfd,buffer,1024,0);
        printf("%s\n",buffer);
        if(strcmp(buffer,"exit")==0)
        {
            printf("you say exeit ...\n   ");
            break;
        }
    }

    return 0;
}