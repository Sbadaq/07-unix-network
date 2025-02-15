
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <arpa/inet.h>

int main()
{
    int serverSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize;
    char buffer[1024]= {0x00};
    struct msghdr msg;
    struct iovec iov;

    if (( serverSocket = socket(AF_INET,SOCK_DGRAM,0)) < 0)
    {
        perror("socket");
        exit(1);
    }
    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverSocket,(struct sockaddr *)&serverAddr,sizeof(serverAddr)) < 0)
    {
        perror("bind");
        exit(1);
    }
    printf("Server is listening on port 12345...\n");

    memset(&msg,0,sizeof(msg));
    msg.msg_name = &clientAddr;
    msg.msg_namelen = sizeof(clientAddr);

    iov.iov_base = buffer;
    iov.iov_len = sizeof(buffer);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;


    while(1)
    {
        clientAddrSize = sizeof(clientAddr);
        int recvBytes  = recvmsg(serverSocket,&msg,0);
        if (recvBytes < 0)
        {
            perror("recvmsg");
            exit(1);
        }
        buffer[recvBytes] = '\0';
        printf("received %d bytes from %s:%d: %s\n",recvBytes,inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port),buffer);
    }
    return 0;
}