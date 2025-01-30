#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 80
#define SERV_PORT 8000

int main(int argc,char** argv)
{
    int sockfd,n;
    char recvline[MAXLINE+1];
    struct sockaddr_in servaddr;
    if(argc!=2)
    {
        perror("usage: a.out <IPaddress>");
        exit(0);
    }    
    //第一步：创建套接字，我认为叫tcp endpoint更加具体些。
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
    {
        perror("socket error");
        exit(0);
    }
    bzero(&servaddr,sizeof(servaddr));

    servaddr.sin_family=AF_INET;
    //第二步：填充服务器地址和端口号。
    servaddr.sin_port=htons(SERV_PORT);
    //inet_pton是将点分十进制形式的IP地址转换为二进制形式。
    inet_pton(AF_INET,argv[1],&servaddr.sin_addr);
    //第三步：发起连接。
    if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0)
    {
        perror("connect error");
        exit(0);
    }
    //第四步：读写套接字。  
    //因为TCP是一个没有边界的字节流协议，如果数据量很大，不能保证一次read就能读完对方发来的数据。
    //所以需要循环读取，直到对方关闭连接（read返回0）或者发生错误(负值）时终止循环。
    while((n=read(sockfd,recvline,MAXLINE))>0)
    {
        recvline[n]='\0';
        //将recvline中的时间戳转换为yyyy-mm-dd hh:mm:ss格式，然后打印到标准输出。
        if(strchr(recvline,':'))
            *strchr(recvline,':')='\0';
        
        if(fputs(recvline,stdout)==EOF)
            perror("fputs error");
    }
    if(n<0)
        perror("read error");
    
    exit(0);
}