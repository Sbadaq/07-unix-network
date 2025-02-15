
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#define MAXLINE 4096
#define DP_POLL 0x5001

     
struct dvpoll{
    struct pollfd *dp_fds;
    int dp_nfds;
    int dp_timeout;
};
     
struct pollfd {
    int fd;         /* 文件描述符 */
    short events;   /* 要监视的事件 */
    short revents;  /* 发生的事件 */
};

void str_cli(FILE* fp,int sockfd)
{
    int stdineof;
    char buf[MAXLINE];
    int n;
    int wfd;
    struct pollfd pfd[2];

    struct dvpoll dpoll;
    int i;
    int result;

    wfd = open("/dev/null",O_RDWR,0);
    pfd[0].fd = fileno(fp);
    pfd[0].events = POLLIN;//POLLIN:等待数据到来
    pfd[0].revents = 0;

    pfd[1].fd = sockfd;
    pfd[1].events = POLLIN;//POLLIN:等待数据到来
    pfd[1].revents = 0;
    stdineof = 0;

    write(wfd,pfd,sizeof(struct pollfd) * 2);//将pfd注册到/dev/poll中，告诉内核我有一个pollfd数组，里面有2个元素
    while(1)
    {
        /*block util /dev/poll say something is ready*/
        dpoll.dp_timeout = -1;//无限等待
        dpoll.dp_nfds = 2;//监视2个文件描述符
        dpoll.dp_fds = pfd;//监视的文件描述符数组
        result = ioctl(wfd,DP_POLL,&dpoll);//等待数据到来
        for(i = 0 ; i < result ; i++)
        {
            if(dpoll.dp_fds[i].fd == sockfd)//如果是socket有数据到来
            {
                if((n = read(sockfd,buf,MAXLINE)) == 0)
                    return;
                else if(write(STDOUT_FILENO,buf,n) != n)
                    err_sys("write error to stdout");
            }
            else if(dpoll.dp_fds[i].fd == fileno(fp))//如果是标准输入有数据到来
            {
                if((n = read(fileno(fp),buf,MAXLINE)) == 0)
                    stdineof = 1;
                else if(write(sockfd,buf,n) != n)
                    err_sys("write error to socket");
            }
        }
    }
}