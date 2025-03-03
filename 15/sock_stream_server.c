 
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

int main()
{
    int listenfd,connfd;
    pid_t child_pid;
    socklen_t clilen;
    struct sockaddr_in cliaddr,servaddr;

}