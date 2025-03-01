

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
 
#include <sys/poll.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
 
#define MAXLINE 4096
#define DP_POLL 0x5001    
struct dvpoll{
    struct pollfd *dp_fds;
    int dp_nfds;
    int dp_timeout;
};
#include <sys/poll.h>
#include <math.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX_WORK_THREAD 1
void str_cli(FILE* fp, int sockfd)
{
    printf("final version !\n");

    int maxfdp1, stdineof;
    fd_set rset;
    char buf[1024];
    int n;

    FD_ZERO(&rset);
    stdineof = 0;

    for(;;)
    {
        if(stdineof == 0)
            FD_SET(fileno(fp),&rset);

        FD_SET(sockfd,&rset);
        maxfdp1 = max(fileno(fp),sockfd) + 1;
        select(maxfdp1,&rset,NULL,NULL,NULL);
        if(FD_ISSET(sockfd,&rset))//is readable
        {
            if(( n = read(sockfd,buf,1024)) == 0)
            {
                if (stdineof == 1)
                {
                    printf("normal termination\n");
                    return; 
                }
                else
                {
                    printf("server terninate prematurely\n");
                    exit(1);
                }
            }
           write(fileno(fp),buf,n);
        }
        if(FD_ISSET(fileno(fp),&rset))//is readable
        {
           if( ( n = read(fileno(fp),buf,1024)) == 0)
           {
                printf("end of file\n");
               stdineof = 1;
               shutdown(sockfd,SHUT_WR);//send FIN
               FD_CLR(fileno(fp),&rset);
               continue;//EOF
           }
           write(sockfd,buf,n);
        }
    }
} 

/**
 * @brief 主函数，用于通过UNIX域套接字进行客户端通信
 *
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 *
 * @return 0 表示程序正常退出
 */
int main(int argc, char **argv)
{
    int sockfd, n;
    char recvline[1024];
    struct sockaddr_un servaddr;
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, argv[1]);
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    str_cli(stdin, sockfd);
    exit(0);

}