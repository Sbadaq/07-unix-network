#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "sum.h"

//这个版本有缺陷，得不到socket断开的及时通知（服务器重启或者崩溃）；
void str_cli_01(FILE *fp, int sockfd)
{
    char send_buf[1024];
    char recv_buf[1024];

    ssize_t nread;
    struct args arg;
    struct result rst;

    while (fgets(send_buf, sizeof(send_buf), fp) != NULL) {
        size_t len = strlen(send_buf);
        if (send_buf[len - 1] == '\n') {
            send_buf[len - 1] = '\0';
        }
        // write(sockfd, send_buf, 1);
        // //for to see SIGPIPE,we write twice
        // sleep(1);
        // write(sockfd, send_buf, len-1);

        if(sscanf(send_buf,"%ld%ld",&arg.arg1,&arg.arg2) != 2) {
            printf("input error\n");
            continue;
        }
        write(sockfd, &arg, sizeof(arg));
        if(read(sockfd, &rst, sizeof(rst)) == 0) {
            printf("the server close socket.\n");
            break;;
        }
        printf("  %ld + %ld = %ld\n", arg.arg1,arg.arg2,rst);
       
        // if((nread = read(sockfd, recv_buf, sizeof(recv_buf))) > 0) {
        //     recv_buf[nread] = '\0';
        //     fputs(recv_buf, stdout);
        //     //printf("recv_buf: %s\n", recv_buf);
        //     fflush(stdout);
        //     printf("\n");
        // }
        // if(nread == 0) {
        //     printf("Server closed the connection\n");
        //     break;
        // }
    }
}
int max(int a, int b) {
    return (a > b) ? a : b;
}
#include <math.h>
#include <sys/select.h>
#include <sys/types.h>
//修订版本，使用select
void str_cli02(FILE* fp, int sockfd)
{
    printf("use select\n");

    int maxfdp1;
    fd_set rset;
    char sendline[1024],recvline[1024];

    FD_ZERO(&rset);

    for(;;)
    {
        FD_SET(fileno(fp),&rset);
        FD_SET(sockfd,&rset);
        maxfdp1 = max(fileno(fp),sockfd) + 1;
        select(maxfdp1,&rset,NULL,NULL,NULL);
        if(FD_ISSET(sockfd,&rset))//is readable
        {
            if(read(sockfd,recvline,sizeof(recvline)) == 0)
            {
                printf("server terminated prematurely\n");
                exit(1);
            }
            fputs(recvline,stdout);
        }
        if(FD_ISSET(fileno(fp),&rset))//is readable
        {
            if(fgets(sendline,sizeof(sendline),fp) == NULL)
                exit(0);
            write(sockfd,sendline,strlen(sendline));
        }
    }
}

//修订版本，解决前一个版本中，不支持多行输入的问题
void str_cli_03(FILE* fp, int sockfd)
{
    printf("use select\n");

    int maxfdp1,stdineof;
    fd_set rset;
    char buf[1024];
    int n;


    FD_ZERO(&rset);

    for(;;)
    {
        if(stdineof == 0)
            FD_SET(fileno(fp),&rset);
        FD_SET(sockfd,&rset);

        maxfdp1 = max(fileno(fp),sockfd) + 1;
        select(maxfdp1,&rset,NULL,NULL,NULL);
        if(FD_ISSET(sockfd,&rset))//is readable
        {
            printf("read from socket is ready\n");
            if(( n = read(sockfd,buf,1024)) == 0)
            {
                if(stdineof == 1)
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
           printf("read from stdin is ready\n");
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

//str_cli 最终版本使用select和shutdown
//这个函数使用了select函数来同时监听套接字和标准输入（或文件）的可读状态，从而实现了非阻塞的双向数据传输。
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

#define MAX_WORK_THREAD 1

int main(int argc, char *argv[], char *envp[])
{
    if(argc !=2)
    {
        perror("Usage: ./a.out <string>");
        return 1;
    }

    int sockfd[MAX_WORK_THREAD];
    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8000);
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
    for(int i = 0 ; i < MAX_WORK_THREAD; i++) {
        if((sockfd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket() failed");
            return -1;
        }
        if (connect(sockfd[i], (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("connect() failed");
            //如果connect函数返回EINTR，则表示connect调用被信号中断。我们不能再次调用 connect函数，因为该调用已经失败了。
            //我们必须使用select函数来等待套接字变为可写，然后再次调用connect函数。
            if (errno == EINTR) {
                printf("connect() was interrupted\n");
            }
            return -1;
        }

        str_cli(stdin, sockfd[i]);
        close(sockfd[i]);

    }
    // if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    //     perror("socket() failed");
    //     return -1;
    // }
    // if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    //     perror("connect() failed");
    //     return -1;
    // }
    // str_cli(stdin, sockfd);
    // close(sockfd);
    exit(0);
}