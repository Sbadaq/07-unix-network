#include <time.h>
#include <stdio.h>
#include <stdlib.h>
//#include <bits/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "sum.h"

void str_echo(int socketfd)
{
    ssize_t n;
    //char buf[1024];
// again:
//     while( (n = read(socketfd, buf, sizeof(buf))) > 0)
//     {
//         write(socketfd, buf, n);
//     }

//     if ( n < 0 && errno == EINTR)
//         goto again;
//     else if ( n < 0 )
//     {
//         printf("write rrror\n"); 
//     }
//     else{
//         printf("client socket closed because read return 0\n");
//     }
    struct args arg;
    struct result rst;
    
    
    for(;;)
    {
        if(( n  = read(socketfd,&arg,sizeof(arg))) == 0)
        {
            printf("client socket closed because read return 0\n");
            break;
        }
        // if(sscanf(buf,"%ld%ld",&arg.arg1,&arg.arg2) == 2)
        // { 
        //     // sprintf(buf,"%ld",arg.arg1 + arg.arg2);
            rst.sum = arg.arg1 + arg.arg2;
             printf("sum is %ld\n",rst.sum);
        // }
        // else{
        //     snprintf(buf, sizeof(buf), "input error \n");
        //     printf("input error\n");
        //     break;;
        // }

        // n = strlen(buf);
        // write(socketfd, buf, n);
        write(socketfd,&rst,sizeof(rst));
    }
}

void sig_chld(int signo)
{
    pid_t pid;
    int stat;
    //pid = wait(&stat);
    while( (pid = waitpid(-1,&stat,WNOHANG)) > 0)
        printf("got SIGCHLD interrupt!  child pid is %d\n",pid);
    return ;
}
int main(int argc,char** argv)
{
    int listenfd,connfd;
    pid_t child_pid;
    size_t len;
    struct sockaddr_in servaddr,cliddr;
    char buff[1024];
    time_t ticks;

    listenfd=socket(AF_INET,SOCK_STREAM,0);
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(8000);
    bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));

    listen(listenfd,10);

    signal(SIGCHLD,sig_chld);

    for(;;)
    {
        printf("i will sleep for %d seconds\n",10);
        sleep(10);
        printf("wake up\n");
        ticks=time(0);
        len=sizeof(cliddr);
        //connfd=accept(listenfd,(struct sockaddr*)&cliddr,&len);
        if( (connfd = accept(listenfd,(struct sockaddr*)&cliddr,&len)) < 0)
        {
            if (errno == EINTR){
                printf("EINTR\n");
                continue;
            }
            else
            {
                printf("errno is %d\n",errno);
                break;
            }
        }
        //printf("connect from:%s:%s\n",inet_ntoa(cliddr.sin_addr),ntohs(cliddr.sin_port));
        if((child_pid = fork() ) == 0){ //child
            close(listenfd);
            str_echo(connfd);
            exit(0);
        }
        close(connfd);//parent
    }

}