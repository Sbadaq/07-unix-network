
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
/**alarm 函数
原理：通过调用 alarm 函数设定一个定时器，当定时器到期时，内核会向进程发送 SIGALRM 信号。在网络编程中，可利用此信号来中断正在进行的 I/O 操作，实现超时控制。
 */
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