## 第一章 简介
### 1.1 网络编程基本步骤
- TCP 编程
    - 服务器端: socket()->bind() -> listen() -> accept() -> read()/write() -> close()
    - 客户端: socket() -> connect() -> read()/write() -> close()
- UDP 编程
    - 服务器端: socket() -> bind() -> recvfrom()/sendto() -> close()
    - 客户端: socket() -> sendto()/recvfrom() -> close()

### 1.2 客户端程序
- 创建 套接字 **socket** 
- 指定 服务器地址和端口 **htons** , **inet_pton** 
- 建立与 服务器连接 **connect**
- 读入并输出 服务器应答 **read**, **write**
- 终止程序 **close**
```c
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

### 1.3 服务器程序
- 创建套接字 **socket**
- 初始化服务器地址 **htons**, **htonl** and **inet_pton**
- 绑定套接字和地址 **bind**
- 把套接字转为监听套接字 **listen**
- 接受客户连接  **accept**
- 读写套接字 **read**, **write**
- 终止连接 **close**
```c
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
### 1.4 字节序
  htons,htonl,ntohs,htonl

## 第二章 传输层
### 2.1 传输层协议
- 传输层功能：提供端到端服务
在网络层之上，增加了可靠性、流量控制、错误检测等功能 
- 主要协议：TCP UDP
UDP 无连接，不可靠，基于数据报。不保证数据报最终到达，不保证先后顺序，也不保证 每个报文 只到达一次,每个UDP 数据报都 有一个长度，而TCP是一个字节流没有边界。

### 2.2 TCP
+ TCP 面向连接，可靠，基于字节流。
+ TCP可靠性：确认、重传、校验和、 
+ TCP 面向连接：三次握手，四次挥手
+ TCP 流量控制：滑动窗口
+ TCP 拥塞控制：慢启动、拥塞避免、快速重传、快速恢复
+ 基于字节流，数据没有边界，需要 程序员自行处理。

1) TCP 头部：源端口、目的端口、序列号、确认号、标志位、窗口大小、校验和、紧急指针。
2) TCP 连接管理 
- 三次握手：客户端发SYN，服务器发SYN/ACK，客户端发ACK
- 四次挥手：客户端发FIN，服务器发FIN/ACK，服务器发ACK，客户端发ACK
3) 状态变换：包括CLOSED、LISTEN、SYN_RCVD、ESTABLISHED、CLOSE_WAIT、LAST_ACK、TIME_WAIT
粗实线表示 客户状态 转换，粗虚线表示 服务器状态 转换

### 2.3 UDP
+ UDP 无连接，不可靠，基于数据报。轻量级。
+ 头部结构：源端口、目的端口、长度、校验和。
+ 适用场景：实时应用、视频会议、广播、多媒体流、DNS、SNMP、TFTP

### 2.4 对比
特性	|TCP	| UDP
---|---|---
连接性	|面向连接	|无连接
可靠性	|可靠（确认、重传、校验和）	|不可靠
数据边界|	基于字节流（无边界）	|基于数据报（有边界）
头部开销|	较大（20字节）	|较小（8字节）
适用场景|	文件传输、Web、电子邮件等	|实时应用、广播、简单查询/响应等

### 2.5 编程
1) TCP 使用SOCK_STREAM类型，调用 connect(),accept()管理连接
* 数据使用send() recv()传输
2) UDP 使用SOCK_DGRAM类型，不需要 连接管理 。
* 调用 sendto(),recvfrom()管理连接

### 2.6 其它但不是不重要的
IPv4 数据报最大65535 
IPv4 最小缓冲区大小  576,因此许多udp应用都 避免 产生大于这个大小 的数据报
应用进程缓冲区，套接字缓冲区
MTU 网络层能传输的最大数据包大小，以太网的MTU为1500
MSS 最大分段大小 MSS=MTU-IP头部-TCP头部
TIME_WAIT 连接关闭后，主动关闭方会进入TIME_WAIT状态，2MSL（最大报文生存时间 ）
确保最后一个ACK到达对端,并防止旧连接的报文 被错误路由到新连接。

## 第三章 套接字
### 3.1 套接字地址结构[重点]
ipv4套接字结构定义 <netinet/in.h>

```c
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
    char sin_zero[8];//not use 
};
```
通用套接字结构定义 <sys/socket.h>
```c
<sys/socket.h>
struct sockaddr{
    uint8_t sa_len;/*address family*/
    sa_family sa_family;/*address type*/
    char sa_data[4];
};
```
int bind(int,struct sockaddr*, socklen_t);
//应用时需要类型强制转换
```c
struct sockaddr_in serv_addr;
bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
```
### 3.2 字节排序
字节排序是指 不同机器之间 字节顺序的不同
- 大端序：高位字节在低地址，低位字节在高地址
- 小端序：低位字节在低地址，高位字节在高地址
网络字节序：大端序

字节排序函数：
```c
#include <netinet/in.h>
uint16_t htons  //host to network short name，16位主机字节转为网络 字节
uint32_t htonl  //host to network long name，32位主机字节转为网络 字节
uint16_t ntohs  //network to host short name，16位网络字节转为主机 字节
uint32_t ntohl  //network to host long name，32位网络字节转为主机 字节
```
### 3.3 地址转换函数
字符串地地址转换二进制地址

```c
#include <string.h>
void *memset
void *memcpy
int memcmp(void *)

inet_aton //点分十进制转换为二进制
inet_ntoa //二进制转换为点分十进制
inet_pton //字符串地地址转换二进制地址
inet_ntop //二进制地址转换为字符串地地址

```
### 3.4 套接字编程基本步骤
* TCP 套接字
1. 创建套接字 **socket**
2. 绑定套接字 **bind**
3. 监听套接字 **listen**
4. 接受连接 **accept**
5. 建立连接 **connect**（客户端）
6. 数据传输 **read**, **write**或者 **send**, **recv**
7. 关闭连接 **close**

* UDP 套接字
1. 创建套接字 **socket**
2. 绑定套接字 **bind**
3. 数据传输 **sendto**, **recvfrom**
4. 关闭连接 **close**

### 3.5 重要概念
+ sockaddr 与 sockaddr_in的转换
通常使用 struct sockaddr作为通用套接字类型，实际使用时需要类型强制转换
将 struct sockaddr_in 转换为 struct sockaddr
+ 地址族
AF_INET ,AF_INET6, AF_UNIX 
+ 端口号
0~65535 16位无符号号数 
## 第四章 TCP套接字编程
### 4.1 套接字函数[重点]
* 服务器端
1. 创建套接字 **socket**
2. 绑定套接字 **bind**
3. 监听套接字 **listen**
4. 接受连接 **accept**
5. 数据传输 **read**, **write**或者 **send**, **recv**
6. 关闭连接 **close**
* 客户端
1. 创建套接字 **socket**
2. 连接服务器 **connect**
3. 数据传输 **read**, **write**或者 **send**, **recv**
4. 关闭连接 **close**

```c
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

### 4.2 TCP生命周期
1. 三次握手
 - 客户端发送SYN
 - 服务器发送SYN/ACK
 - 客户端发送ACK
2. 四次挥手
 - 一方发送FIN
 - 另一方发送FIN/ACK
 - 另一方发送ACK
 - 一方发送FIN

 ### 4.3 地址结构
 - 通用地址结构
 ```c
 struct sockaddr{
    uint8_t sa_len;/*address family*/
    sa_family sa_family;/*address type*/
    char sa_data[4];
};
```
- ipv4 地址结构
```c
struct sockaddr_in{
    uint8_t sin_len;
    sa_family_t sa_family; /*AF_INET, AF_INET6, AF_
    in_port_t in_port; /* 16 bit port */
    struct in_addr sin_addr;/*32 bit address*/
    char sin_zero[8];//not use
};
```
### 4.4 获取地址信息
```c
#include <netdb.h>
struct hostent *gethostbyname(const char *name);
int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```
### 4.5 fork 和exec使用场景 
- 并发服务器
 在服务器中，父进程调用fork创建子进程，子进程处理每个客户端请求，父进程继续监听新的客户端连接。
- 多任务处理
 服务器使用fork创建多个子进程，每个子进程处理一个任务。
- 守护进程
 通过fork创建守护进程，父进程退出，子进程成为守护进程，继续执行任务。

- exec 函数族
 调用exec函数族中的函数会替换当前进程的映像，执行新的程序。
 进程id不变。
```c
#include <unistd.h>
int execl(const char *path, const char *arg, ...);
int execlp(const char *file, const char *arg,...);
int execle(const char *path, const char *arg,...);
int execv(const char *path, char *const argv[]);
```
    - 进程替换
    - 脚本解释器
    - 动态加载程序
    - 系统调用
- 典型场景 fork 和exec结合使用 
    在子进程中fork新进程，然后调用exec函数执行新的程序。
    示例：实现并发和动态加载 

## 第五章 TCP客户服务器示例
### 5.1 本章目标
- 通过一个完整的TCP客户服务器程序，掌握TCP套接字编程的基本步骤。
- 理解服务器和客户端的设计与实现
- 学习如何处理常见的网络编程问题，如僵尸进程和信号处理。

### 5.2 服务器端[重点]
1. 创建套接字 **socket**
```c
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
```
2. 绑定套接字 **bind**
```c
struct sockaddr_in servaddr;
servaddr.sin_family = AF_INET;
servaddr.sin_port = htons(SERV_PORT);
servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
```
3. 监听套接字 **listen**
```c
listen(sockfd, 5);
```
4. 接受连接 **accept**
```c
struct sockaddr_in cliaddr;
socklen_t cliaddr_len = sizeof(cliaddr);
int connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
```
5. 数据传输 **read**, **write**或者 **send**, **recv**
```c
char buf[MAXLINE];
int n = read(connfd, buf, MAXLINE);
write(connfd, buf, n);
```
6. 关闭连接 **close**
```c
close(sockfd);
```
### 5.3 客户端[重点]
1. 创建套接字 **socket**
2. 连接服务器 **connect**
```c
struct sockaddr_in servaddr;
servaddr.sin_family = AF_INET;
servaddr.sin_port = htons(SERV_PORT);
inet_pton(AF_INET,"127.0.0.1",&servaddr.sin_addr);
connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
```
3. 数据传输 **read**, **write**或者 **send**, **recv**
```c
char buf[MAXLINE];
int n = read(sockfd, buf, MAXLINE);
write(sockfd, buf, n);
```
4. 关闭连接 **close**

### 5.4 信号处理
信号就是软中断, signal 通常是异步发生的，程序不知道 信号发生时刻 。
- 处理**SIGCHLD**信号，避免僵尸进程
- 处理**SIGPIPE**信号，防止客户端断开连接后 服务器崩溃
### 5.5 僵尸进程
- 什么是僵尸进程？
  子进程终止，父进程没有调用wait或waitpid获取子进程的状态信息，子进程进入僵尸状态。
- 如何避免？
  无论何时fork 都 需要 wait或者 waitpid 清理僵尸进程。
```c
signal(SIGCHLD, sig_chld);//注册信号处理函数
void sig_chld(int signo) {
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);
    return;
}
```
### 5.6 并发服务器
使用fork实现并发服务器
```c
if (fork() == 0) { //child is already
    //子进程    
    close(listenfd);//关闭监听套接字
    handle_client(connfd);//处理客户请求
    close(connfd);//关闭连接套接字
    exit(0);//子进程 退出
}
close(connfd);//父进程关闭连接 套接字（但是保留监听套接字）
```

*解释下面的函数原型*
```c
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

