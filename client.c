#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "sum.h"
void str_cli(FILE *fp, int sockfd)
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