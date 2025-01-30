## 第一章 简介
*客户端程序*
- 创建 套接字
- 指定 服务器地址和端口
- 建立与 服务器连接
- 读入并输出 服务器应答
- 终止程序
```
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
    //所以需要循环读取，直到对方关闭连接（read返回0）或者发生错误（负值）时终止循环。
    while((n=read(sockfd,recvline,MAXLINE))>0)
    {
        recvline[n]='\0';
        if(fputs(recvline,stdout)==EOF)
            perror("fputs error");
    }
    if(n<0)
        perror("read error");
    
    exit(0);
}
```

*服务器程序*
- 创建套接字
- 公开端口绑定到套接字
- 把套接字转为监听套接字
- 接受客户连接，发送应答
- 终止连接
```
int main(int argc, char *argv[]) {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    char buf[MAXLINE];
    time_t ticks;

     
    // 第一步：创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // 第二步：初始化服务器地址
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //第三步： 绑定套接字和地址（被动打开）
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    //第四步：把套接字转换成监听套接字，准备接受连接请求
    listen(sockfd, 5);
    printf("Server is running...\n");
    while (1) {
        cliaddr_len = sizeof(cliaddr);
        //第五步： 接受连接请求
        connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
        printf("Received a connection from %s\n", inet_ntoa(cliaddr.sin_addr));
        //第六步： 收发数据
        ticks = time(NULL);
        snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
        write(connfd, buf, strlen(buf));
        //第7步： 关闭连接
        close(connfd);
    }
    return 0;
}
```

## 第二章 传输层
UDP 不保证数据报最终到达，不保证先后顺序，也不保证 每个报文 只到达一次
每个UDP 数据报都 有一个长度，而TCP是一个字节流没有边界。
TCP 提供可靠性传递或故障的可靠通知，流量控制
TCP 三次握手，四次挥手
状态变换 图：粗实线表示 客户状态 转换，粗虚线表示 服务器状态 转换

IPv4 数据报最大65535 
IPv4 最小缓冲区大小  576,因此许多udp应用都 避免 产生大于这个大小 的数据报
应用进程缓冲区，套接字缓冲区

# 第二部分 基本套接字
## 第三章 套接字
```
<netinet/in.h>
<sys/types.h>
struct in_addr {
    in_addr_t s_addr;/*32 bit address*/
};
struct sockaddr_in {
    uint8_t sin_len;
    sa_family_t sa_family; /*AF_INET, AF_INET6, AF_
    in_port_t in_port; /* 16 bit port */
    struct in_addr sin_addr;/*32 bit address*/
    char sin_zero[8];
};
<sys/socket.h>
struct sockaddr{
    uint8_t sa_len;/*address family*/
    sa_family sa_family;/*address type*/
    char sa_data[4];
};
```
int bind(int,struct sockaddr*, socklen_t);
//应用时需要类型强制转换
```
struct sockaddr_in serv_addr;
bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
```
字节排序函数
```
#include <netinet/in.h>
uint16_t htons  //host to network short name
uint32_t htonl  //host to network long name
uint16_t ntohs  //network to host short name
uint32_t ntohl  //network to host long name
```
字节函数
```
#include <string.h>
void *memset
void *memcpy
int memcmp(void *)

inet_aton
inet_pton //presentation to numeric address
inet_ntop //numerical address to presentation

```

## 第四章 TCP套接字编程
```
#include <sys/socket.h>
int socket(int family, int type, int protocol);//成功返回 非负描述符
int connect(int sockfd,const struct sockaddr *server,socklen_t server_len);//成功返回 0 
int bind(int sockfd,const struct sockaddr* myaddr,socklen_t mylen);//成功返回 0
int listen(int sockfd,int backlog);//成功返回 0
int accept(int sockfd, struct sockaddr* cliaddr,socklen_t *addrlen);

int gethostname
int getpeername

#include <unistd.h>
pid_t fork();
int close(int sockfd);
```




## 第五章 TCP客户服务器示例
信号就是软中断, signal 通常是异步发生的，程序不知道 信号发生时刻 。
无论何时fork 都 需要 wait
没有办法防止wait在正运行的子进程 尚有未终止的阻塞
所以 必须调用 waitpid


*解释下面的函数原型*
```
void (*signal(int signo, void (*func)(int)))(int);
```
1. 函数整体结构：函数名为signal,signal函数返回一个指向 函数的指针 ，这个被指向的函数接受 一个int类型参数 并返回 void
2. 返回类型：返回类型是void(*)(int); 即一个指向函数的指针 ，该函数接受一个int参数并返回void；注意 函数名本身 不是指针，返回类型是指针。
3. 参数列表：第2个参数 是一个函数指针 ，指向一个接受int并返回void的函数。


*发送二进制数据（结构体）*
发送数据内容是01 01，下面的字节顺序是小端还是大端？
```
15:40:57.637154 IP (tos 0x0, ttl 64, id 65265, offset 0, flags [DF], proto TCP (6), length 68)
    127.0.0.1.55770 > 127.0.0.1.8000: Flags [P.], cksum 0xfe38 (incorrect -> 0xf634), seq 16:32, ack 9, win 512, options [nop,nop,TS val 4055805360 ecr 4055788692], length 16
	0x0000:  4500 0044 fef1 4000 4006 3dc0 7f00 0001  E..D..@.@.=.....
	0x0010:  7f00 0001 d9da 1f40 f845 5c78 7fab b226  .......@.E\x...&
	0x0020:  8018 0200 fe38 0000 0101 080a f1be adb0  .....8..........
	0x0030:  f1be 6c94 0100 0000 0000 0000 0200 0000  ..l.............
	0x0040:  0000 0000
```
    在小端字节序（Little Endian）中，较低的字节存储在内存的较低地址处，而较高的字节存储在较高的地址处。
因此，一个 4 字节的 long 类型值 01（在十六进制中表示为 0x00000001）会按如下方式存储：
```
 0100 0000 0000 0000
 0200 0000 0000 0000
```
我们必须在客户端使用select或poll函数

## 第六章 select和poll函数
C/C++异步IO的开源框架有：AIO,Workflow,libuv,libevent,boost.asio,muduo


## 第七章 套接字选项

## 第八章 基本UDP

## 第九章 基本sctp

## 第十章 sctp

## 第十一 名字与地址


# 第三部分 高级套接字
## 第十二 ipv6 

## 第十三 守护进程 

## 第十四 高级 io

## 第十五 域协议 

## 第十六 非阻塞 io

## 第十七 ioctl

## 第十八 路由套接字

## 第十九 密钥管理 

## 第二十 广播

## 第二一 多播

## 第二二 高级 UDP

## 第二三 高级 sctp

## 二十四 带外

## 二十五 信号驱动

## 二十六 线程

## 二十七 IP

## 二十八 原始套接字

## 二十九 数据链路访问 

## 三十章 客户服务器设计模式 

## 三十一 流

