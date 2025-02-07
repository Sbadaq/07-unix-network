
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

static void connect_alarm(int signo)
{
    printf("alarm : timeout\n");
    exit(EXIT_FAILURE);
}

int set_timeout_with_alarm(int sockfd)
{
    signal(SIGALRM,connect_alarm);    
    alarm(5);

    char buf[1024];
    ssize_t recv_len = recv(sockfd,buf,sizeof(buf),0);
    alarm(0);

    if(recv_len == -1){
        if(errno == EINTR)
            return -1;
        else
            return -1;
    }else{
        buf[recv_len] = '\0';
        printf("connect success\n");
    }
}


int main()
{
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET,"127.0.0.1",&servaddr.sin_addr);
    servaddr.sin_port = htons(8080);

    connect(sockfd,(const struct sockaddr *)&servaddr,sizeof
    (servaddr));

    set_timeout_with_alarm(sockfd);
    return 0;

}