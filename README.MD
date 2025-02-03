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
- wait 和waitpid区别
都用于等待子进程终止，但是waitpid比wait更灵活。
- 当父进程不关系是哪个子进程先结束，只需要在有子进程结束时进行资源回收，使用wait比较合适。
- 当父进程需要知道子进程的状态信息，或者需要在子进程结束后进行一些清理工作，使用waitpid比较合适。
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


***nmap***
nmap is an implementation of the port scanning technique.
它主要侧重于对网络 中主机和端口进行全面 的探测和分析 ，以帮助网络管理员 识别网络中的安全漏洞、开放端口、服务等信息。甚至可以 根据扫描结果推测出目标主机的操作系统、服务等信息。
+ nmap 扫描的方式有：
    1. 端口扫描（Port Scanning）：nmap 可以通过向目标主机的指定端口发送 SYN 数据包来进行端口扫描。如果目标端口开放，则会收到一个 SYN/ACK 响应，否则会收到一个 RST 响应。
    2. 服务识别（Service Identification）：nmap 可以通过向目标主机的指定端口发送特定的数据包来识别目标主机上运行的服务。
    3. 操作系统识别（Operating System Identification）：nmap 可以通过向目标主机的指定端口发送特定的数据包来识别目标主机上运行的操作系统。

+ nmap 扫描的过程：
    1. 发送 SYN 数据包到目标主机的指定端口。
    2. 接收响应数据包。
    3. 根据响应数据包的内容判断目标端口的状态。

+ nmap 扫描的结果：
    1. 开放端口：目标端口开放，nmap 会显示目标端口的状态为 open。
    2. 关闭端口：目标端口关闭，nmap 会显示目标端口的状态为 closed。
    3. 过滤端口：目标端口被防火墙过滤，nmap 会显示目标端口的状态为 filtered。
    4. 未知端口：目标端口的状态未知，nmap 会显示目标端口的状态为 unknown。

***fuser***
fuser 是一个用于显示进程使用文件的工具。它可以显示指定文件的所有进程，以及这些进程使用的文件类型。
侧重于本地系统中进程与端口的关联关系 ，能让用户快速定位到占用某个端口的进程，以便进行调试和优化。
基于操作系统的进程管理机制和文件 系统的相关信息工作，通过读取/proc文件系统中的信息来获取进程与端口的关联关系。

## 第六章 IO复用：select和poll函数
### 6.1 概述
```
我们在第5章中的客户端程序中，使用了标准输入和tcp套接字。遇到的难题是，客户阻塞于fgets调用期间 ，服务进程会被杀死，虽然服务进程会正确地给客户发一个fin
，但客户依然阻塞在标准输入中，它将看不到这个eof; 直到从套接字读取为止，这已经 过去了很长时间。

这就需要一种能力，预先告知内核的能力。使得内核一旦发现进程指定的一个或多个io条件就绪，也就是说输入已经 准备好，或描述符已经 能承接更多的输出，它就通知进程 。
这个能力 称为IO复用。
```
io复用典型场合：
- 当客户处理多个描述符时，必须使用多路复用
- 一个客户同时处理多个套接字
- 服务器要处理多个网络 连接
- 服务器要同时处理TCP和UDP
- 服务器要同时处理多个协议

### 6.2 IO模型
unix下五种可用的io模型基本区别:
1. 阻塞IO ： 默认阻塞，最流行的IO模型，前5章我们用的都 是这种。
2. 非阻塞IO ： 不阻塞，进程把自己设置成非阻塞时是在通知内核，当所请求的io操作 不能进行时，它返回一个错误EWOULDBLOCK。
3. IO复用(select/poll): 阻塞在这2个系统调用中的一个之上，而不是真正阻塞在io系统调用上。使用io复用的优势在于可以 等待多个描述符。
4. 信号驱动IO ： 让内核在描述符就绪时 发送一个SIGIO信号，进程接收到信号后 再进行IO操作。
5. 异步IO：异步IO模型是真正的异步IO模型。信号驱动是由内核通知我们可以 启动一个io,而异步io是由内核通知我们io操作何时完成 。

区别：
- 同步io : 导致请求阻塞，直到io完成。
- 异步io : 不阻塞进程 

```c
struct timeval{
    long tv_sec;
    long tv_usec;
};
```
描述符就绪条件 ：
1. 数据可读，满足下面4个条件中的一个：
    - 该套接字接收缓冲区数据字节数大于等于套接字接收缓冲区低水位标记SO_RCVLOWAT。
    - 该连接的读半部关闭（接收了fin的tcp），此时读操作将不阻塞并返回0(也就是eof)。
    - 该套接字上有一个读错误。这样的套接字不阻塞并返回一个-1
    - 该套接字是一个监听套接字，并且已完成的连接数大于0。这样的套接字的accept通常不会阻塞。

2. 数据可写，满足下面4个条件之一：
    - 该连接发送缓冲区的可用字节数，大于等于发送缓冲区低水位标记SO_SNDLOWAT。并且套接字已连接，或者不需要 连接(udp).
    - 该连接的写半部关闭（发送了fin的tcp）。对这样的套接字，继续写操作将返回SIGPIPE信号。
    - 使用非阻塞的套接字已经建立连接，或者connect已经以失败返回。
    - 其中有一个错误待处理，写操作将不阻塞并立即返回-1.

3. 如果一个套接字有带外数据或仍处于带外标记，它有异常条件待处理。

### 6.3 系统调用
终止网络连接的方法通常是使用close,但是它有2个限制，却可以 使用shutdown来避免。
(1). close会把引用计数减1，当引用计数为0时，才会真正关闭连接。
(2). close会终止读和写2个方向 的连接，但既然tcp是全双工的，那么它的读半部和写半部都可以关闭。
shutdown函数可以 控制读、写或2个方向的连接。
```c
#include <sys/socket.h>
int shutdown(int sockfd, int howto);
//howto :
//SHUT_RD :关闭连接的读半部
//SHUT_WR :关闭连接的写半部
//SHUT_RDWR :关闭连接的读半部和写半部
```
 
- IO多路复用
  允许程序在单个线程内同时监听多个文件 描述符的状态，从而对多个IO操作的高效管理，避免使用多线程或多进程的额外开销。
- 函数原型
```c
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
//nfds :待测试的最大文件 描述符加1
//readfds :监控读事件的文件描述符集合
//writefds :监控写事件的文件描述符集合
//exceptfds :异常事件文件描述符集合
//timeout :设置select等待的超时时间
    - 返回值
        - 大于0 : 表示有文件 描述符状态发生变化的数量
        - 等于0 : 表示超时，没有文件 描述符变化
        - 小于0 : 表示出错，同时设置errno表示错误原因
 
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
//fds :指向一个结构体数组，每个元素包含一个文件描述符和一个事件掩码
//nfds :fds数组的大小
//timeout :设置poll等待的超时时间
 
    - 返回值    
        - 大于0 : 表示有文件 描述符状态发生变化的数量
        - 等于0 : 表示超时，没有文件 描述符变化
        - 小于0 : 表示出错，同时设置errno表示错误原因
```
- 对比
1.数据结构：select使用fd_set存储文件描述符集合，poll使用pollfd结构体数组存储文件描述符和事件掩码。后者更加直观。
2.文件描述符数量：select支持的最大文件描述符数量为1024，poll没有限制。
3.事件通知：select返回的是一个文件描述符集合，需要遍历集合找到发生变化的文件描述符。poll返回的是一个结构体数组，每个元素包含一个文件描述符和一个事件掩码，直接可以找到发生变化的文件描述符。

### 6.4 难点知识点
#### （一）文件描述符集合操作
1.select使用FD_ZERO、FD_SET、FD_CLR、FD_ISSET等宏操作文件描述符集合。容易出错，需要 注意每次调用select前都要重新初始化集合。
2.理解pollfd结构体中events和revents字段的含义。events表示用户关心的事件，revents表示实际发生的事件。
#### （二）超时时间
1.理解select和poll的超时时间参数的含义。select的timeout参数是一个struct timeval结构体，poll的timeout参数是一个毫秒数。
2.理解select和poll的返回值含义。select返回的是文件描述符集合中发生变化的文件描述符数量，poll返回的是结构体数组中发生变化的元素数量。

#### （三）典型示例
1.实现一个简单的select服务器。
```c
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

int main()
{
    int listen_fd,conn_fd;
    struct sockaddr_in server_addr,client_addr;
    socklen_t cli_len = sizeof(server_addr);
    fd_set read_fds,temp_fds;
    int max_fd;

    //第一步
    listen_fd = socket(AF_INET,SOCK_STREAM,0);
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9876);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //第二步
    bind(listen_fd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    listen(listen_fd,MAX_CLIENTS);
    //第三步
    FD_ZERO(&read_fds);
    FD_SET(listen_fd,&read_fds);
    max_fd = listen_fd;

    while(1){
        
        temp_fds = read_fds;
        int ret = select(max_fd+1,&temp_fds,NULL,NULL,NULL);
        if(ret > 0){
            //第四步
            printf("Select returned above zero\n");
            if(FD_ISSET(listen_fd,&temp_fds)){
                conn_fd = accept(listen_fd,(struct sockaddr *)&client_addr,&cli_len);
                FD_SET(conn_fd,&read_fds);
                if(conn_fd > max_fd){
                    max_fd = conn_fd;
                }
            }
            for(int i = listen_fd + 1; i <= max_fd; i++){
                if(FD_ISSET(i,&temp_fds)){
                    char buffer[BUFFER_SIZE];
                    memset(buffer,0,BUFFER_SIZE);
                    ret = read(i,buffer,BUFFER_SIZE);
                    if(ret == 0){
                        printf("Client disconnected\n");
                        close(i);
                        FD_CLR(i,&read_fds);
                    }
                    else{
                        buffer[ret] = '\0';
                        printf("Received message: %s\n",buffer);
                        send(i,buffer,ret,0);
                    }
                }
            }
        }
    }
    close(listen_fd);
    return 0;
}

```
2.使用poll实现一个udp服务器
```c
int main()
{
    //1.创建套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    //2.初始化服务器地址
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9876);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //3.绑定套接字和地址
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    //4.初始化pollfd结构体数组
    struct pollfd fds[1];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    //5.调用poll函数等待事件发生
    int ret = poll(fds, 1, -1);
    //6.如果udp套接字可读，则接收数据并回复
    if (ret > 0)
    {
        if (fds[0].revents & POLLIN)
        {
            char buf[1024];
            struct sockaddr_in cliaddr;
            socklen_t cliaddr_len = sizeof(cliaddr);
            int n = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &cliaddr_len);
            if (n > 0)
            {
                printf("Received: %s\n", buf);
                sendto(sockfd, buf, n, 0, (struct sockaddr *)&cliaddr, cliaddr_len);
            }
        }
    }
    //7.关闭套接字
    close(sockfd);
    return 0;
}
```
#### elect和poll区别
1. 数据结构：select使用fd_set存储文件描述符集合，poll使用pollfd结构体数组存储文件描述符和事件掩码。后者更加直观。
2. 文件描述符数量：select支持的最大文件描述符数量为1024，poll没有限制。
3. 效率：select的效率比poll低，因为每次调用select都需要遍历整个文件描述符集合。
4. 修改文件 描述符：select需要每次调用select前都要重新初始化集合，而poll可以直接修改结构体数组。 
5. 返回值：select 只会返回发生变化的文件描述符数量，而poll返回的是结构体数组中发生变化的元素数量。

#### 我该用哪个？
1. 如果需要支持的文件描述符数量较少，可以使用select。
2. 如果需要支持的文件描述符数量较多，可以使用poll。

C/C++异步IO的开源框架有：AIO,Workflow,libuv,libevent,boost.asio,muduo


## 第七章 套接字选项
### 7.1 概述
有很多方法 来影响套接字的行为。
- getsockopt 和setsockopt函数
- ioctl函数
- fcntl函数 设置非阻塞模式、设置信号驱动IO、设置属主的方法函数

### 7.2 获取和设置套接字选项的函数
```c
#include <sys/socket.h>
int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
//level is the protocol level at which the option resides
    - SOL_SOCKET：通用套接字选项。
    - IPPROTO_IP：IPv4套接字选项。
    - IPPROTO_TCP：TCP套接字选项。
    - IPPROTO_IPV6：IPv6套接字选项。
```
**常用选项**
#### SO_REUSEADDR* 允许重用本地地址 
  这个条件通常是这样遇到的：
 1. 启动一个监听服务器
 2. 连接请求到达，派生一个进程来处理客户请求
 3. 服务器进程终止，但是客户请求还没有被处理完。
 4. 服务器进程重启，但是新的服务器进程无法绑定到相同的本地地址和端口，因为该地址和端口已经被前一个服务器进程占用。
 5. 解决方案是设置SO_REUSEADDR选项。
***所有TCP服务器，调用bind之前，都应该设置这个选项***
***在编写一个可在同一时刻在同一主机上运行多次的多播应用时，设置SO_REUSEADDR选项是很有用的***

#### *SO_REUSEPORT* 允许重用本地地址和端口
  对于tcp,我们绝不可能绑定相同ip和相同port的2个套接字。

#### *SO_KEEPALIVE* 保持连接 
    如果在2小时内任一方没有数据交换，就给对端发一个保持存活的探测分节。
#### *SO_LINGER* 
本选项是指定tcp协议 的，而不是udp; 关闭连接时的行为；
本选项要求用户 进程 与内核之间 传递如下数据结构：
``c
struct linger{
    int l_onoff;
    int l_linger;
};

``` 
    (1) 如果传递{0,0} 则关闭这个选项，close是默认行为。只是将数据从本地缓冲区发送出去。
    (2) 如果传递{1,0} 则没有四步挥手的动作。然而，TIME_WAIT是我们的朋友，应该弄清楚它，而不是避免这个状态。
    (3) 如果传递{1,非0} 则指定了一个延迟时间 ，这种情况下，close等它的数据和fin被服务器的tcp确认了再返回 。
        - close成功返回 只是告诉我们先前发送的数据和fin已经由对端tcp确认。而不是告诉我们对端的应用进程是否已经读取数据。
        如果不设置这个套接字选项，那我们连对端tcp是否确认了都不知道 。
        - 让客户知道服务器已经读取数据的一个方法 是改为调用 shutdown 并设置第2个参数 为SHUT_WR而不是调用 close ,并等待对端close连接的当地端(服务器端);   
        - 获取对端应用已经读取我们数据的另一个方法是应用级确认。简称应用ACK。
 
```c
char ack;
write(sockfd,data,mbytes);
n = read(sockfd,&ack,1);
...
nbytes = read(sockfd,buff,sizeof(buff));
write(sockfd,"",1);
```


#### **SO_ERROR** 可以获取但不可以 设置 的选项


#### 流量控制 
*SO_RCVBUF* 接收缓冲区大小 ;必须在connect之前设置 。
*SO_SNDBUF* 发送缓冲区大小
套接字缓冲区限定了通告对端的窗口大小 ，接收缓冲区就不可能溢出。

**SO_RCVLOWAT** 接收低水位
**SO_SNDLOWAT** 发送低水位  ，因为udp并不为发送数据保留副本，所以总是有数据就可以写，没有发送缓冲区。

### 7.3 传输效率
- 带宽延迟积
  指网络带宽与往返延迟的乘积。表示一个数据从发送端到接收端，再返回发送端的往返时间内，网络能容纳的数据量。
  例如，网络带宽是10Mbps,往返延迟是100ms，那么带宽延迟积就是10Mbps * 100ms = 1Mbps。
- 长胖管道
  指一个管道的容量。表示一个数据从发送端到接收端，再返回发送端的往返时间内，管道能容纳的数据量。由于延迟积大，数据从发送到接收到确认的时间 长，导致管道内存在大量的未确认状态，需要更多机制保证效率和可靠性。

tcp套接字缓冲区大小必须为mss的整数4倍。
管道的容量称为 带宽延迟积;

### 7.4 fcntl函数
fcntl 提供的网络特性:
- 非阻塞io 
    通过使用F_SETFL 命令设置O_NONBLOCK文件状态标志，可以将一个套接字设置为非阻塞模式。
- 信号驱动io
    通过使用F_SETFL 命令设置O_ANSYC文件状态标志，可以将一个套接字设置为信号驱动模式。
- 属主
    通过使用F_GETOWN 和F_SETOWN 命令，可以获取和设置一个套接字的属主。

开启非阻塞io典型代码是
```c
 int flags;
 //set a socket to nonblocking mode
 if (( flags = fcntl(sockfd, F_GETFL, 0)) < 0)
    err_sys("fcntl F_GETFL error");
 flags |= O_NONBLOCK;
 if (fcntl(sockfd, F_SETFL, flags) < 0)
    err_sys("fcntl F_SETFL error");
```


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

