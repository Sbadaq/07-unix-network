#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <errno.h>

int main()
{
    int sockfd =socket(AF_INET,SOCK_SEQPACKET,IPPROTO_SCTP);
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(18080);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
    listen(sockfd,5);

    struct sctp_sndrcvinfo sri;
    while (1)
    {
        int connfd = accept(sockfd,NULL,NULL);
        if(connfd < 0)
        {
            fprintf(stderr, "errno: %d\n", errno);
            exit(1);
        }
        
        memset(&sri,0,sizeof(sri));
        char buffer[1024];

        int n = sctp_recvmsg(connfd, buffer, 1024, NULL, 0, &sri, 0);
        if (n < 0) {
            perror("sctp_rcvmsg");
            close(connfd);
            continue;
        }
        sri.sinfo_stream = 1;
        n = sctp_sendmsg(connfd, NULL, 0, "Hello", 5, 0, 0, sri.sinfo_stream, 0, 0);
        if (n < 0) {
            perror("sctp_sndmsg");
            close(connfd);
            continue;
        }
        
    }
    
}