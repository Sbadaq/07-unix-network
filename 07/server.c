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

void str_echo_sum(int socketfd)
{
    ssize_t n;
    struct args arg;
    struct result rst;
    
    
    for(;;)
    {
        if(( n  = read(socketfd,&arg,sizeof(arg))) == 0)
        {
            printf("client socket closed because read return 0\n");
            break;
        }
            rst.sum = arg.arg1 + arg.arg2;
             printf("sum is %ld\n",rst.sum);
        write(socketfd,&rst,sizeof(rst));
    }
}

void str_echo(int socketfd)
{
    ssize_t n;
    char buf[1024];
again:
    while( (n = read(socketfd, buf, sizeof(buf))) > 0)
    {
        buf[n] = '\0';
        printf("read from client: %s\n",buf);
        write(socketfd, buf, n);
    }

    if ( n < 0 && errno == EINTR)
        goto again;
    else if ( n < 0 )
    {
        printf("write rrror\n"); 
    }
    else{
        printf("client socket closed because read return 0\n");
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
int main01(int argc,char** argv)
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
       // printf("i will sleep for %d seconds\n",10);
       // sleep(10);
       // printf("wake up\n");
       // ticks=time(0);
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
        char addr_str[INET_ADDRSTRLEN];
        printf("connect from:%s:%hu\n",inet_ntop(AF_INET, &cliddr.sin_addr, addr_str, sizeof(addr_str)),ntohs(cliddr.sin_port));
        //printf("connect from:%s:%hu\n",inet_ntoa(cliddr.sin_addr),ntohs(cliddr.sin_port));
        if((child_pid = fork() ) == 0){ //child
            close(listenfd);
            str_echo(connfd);
            exit(0);
        }
        close(connfd);//parent
    }

}

 
#include <sys/select.h>

int main(int argc,char** argv)
{
    int i,maxi, maxfd,listenfd,connfd,sockfd;
    int nready,client[FD_SETSIZE];

    ssize_t n;
    fd_set rset,allset;
    struct sockaddr_in cliaddr,servaddr;
    char buf[1024];
    //  创建socket
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    //初始化服务端地址
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(8000);
    //绑定socket到服务器地址
    bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    //监听
    listen(listenfd,10);
    //初始化最大文件描述符和客户端数组
    maxfd = listenfd;/**initialize */
    maxi=-1;        /** index into client array */
    for(i=0;i<FD_SETSIZE;i++)
        client[i] = -1;   /** -1 indicate available entry */
    //初始化文件描述符集合
    FD_ZERO(&allset);
    FD_SET(listenfd,&allset);


    for(;;)
    {
        rset=allset;
        //使用select等待文件描述符变化
        nready=select(maxfd+1,&rset,NULL,NULL,NULL);
        //如监听文件描述符就绪
        if((sockfd=FD_ISSET(listenfd,&rset))){
            int size=sizeof(cliaddr);
            //接受连接
            connfd = accept(listenfd,(struct sockaddr*)&cliaddr,&size);
            //寻找一个空闲的客户端数组位置
            for(i = 0; i <= FD_SETSIZE; i++)
                if(client[i] == -1){
                    client[i]=connfd;
                    break;
                }
            //没有空闲索引
            if(i == FD_SETSIZE-1)
                printf("too many clients\n");
            //将连接加入文件描述符集合
            FD_SET(connfd,&allset);
            //更新最大文件描述符
            if(connfd>maxfd) maxfd=connfd;
            //更新最大索引
            if(i>maxi) maxi = i;
            //如果没有其它的文件描述符，进行下次循环
            if(--nready <= 0) continue; 
        }
        //遍历所有客户端检查是否有数据
        for(i = 0; i <= maxi; i++){ //check all clients for data
            //如果文件描述符为0 则跳过
            if((sockfd=client[i])<0) continue;
            //如果文件描述符就绪
            if(FD_ISSET(sockfd,&rset)){
                //读取数据
                if((n = read(sockfd,buf,sizeof(buf))) <= 0){
                    //关闭文件描述符，从集合中移除
                    close(sockfd);
                    FD_CLR(sockfd,&allset);
                    client[i]=-1;
                }else{
                    //将取到的数据回写
                    write(sockfd,buf,n);
                }
                //如果没有其它的文件描述符，跳出循环
                if(--nready<=0) break;
            }
        }
    }
}