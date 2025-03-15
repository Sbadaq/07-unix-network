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

>所有TCP服务器，调用bind之前，都应该设置这个选项
>在编写一个可在同一时刻在同一主机上运行多次的多播应用时，设置SO_REUSEADDR选项是很有用的 

#### *SO_REUSEPORT* 允许重用本地地址和端口
  对于tcp,我们绝不可能绑定相同ip和相同port的2个套接字。

#### *SO_KEEPALIVE* 保持连接 
    如果在2小时内任一方没有数据交换，就给对端发一个保持存活的探测分节。
#### *SO_LINGER* 
本选项是指定tcp协议 的，而不是udp; 关闭连接时的行为；
本选项要求用户 进程 与内核之间 传递如下数据结构：

```c
struct linger{
    int l_onoff;
    int l_linger;
};

``` 
>1. 如果传递{0,0} 则关闭这个选项，close是默认行为。只是将数据从本地缓冲区发送出去。
>2. 如果传递{1,0} 则没有四步挥手的动作。然而，TIME_WAIT是我们的朋友，应该弄清楚它，而不是避免这个状态。
>3. 如果传递{1,非0} 则指定了一个延迟时间 ，这种情况下，close等它的数据和fin被服务器的tcp确认了再返回 。
>   - close成功返回 只是告诉我们先前发送的数据和fin已经由对端tcp确认。而不是告诉我们对端的应用进程是否已经读取数据。
>   如果不设置这个套接字选项，那我们连对端tcp是否确认了都不知道 。
>   - 让客户知道服务器已经读取数据的一个方法 是改为调用 shutdown 并设置第2个参数 为SHUT_WR而不是调用 close ,并等待对端close连接的当地端(服务器端);   
>   - 获取对端应用已经读取我们数据的另一个方法是应用级确认。简称应用ACK。
 
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
### 8.1 概述
udp 是无连接不可靠的数据报协议

```c
struct sockaddr_in servaddr;
memset(&servaddr, 0, sizeof(servaddr));
servaddr.sin_family = AF_INET;
servaddr.sin_port = htons(1234);
inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

```
recvfrom和sendto函数，类似于标准的read和send函数，不过需要三个额外的参数。
```c
//sendto函数用于发送数据，可指定目标地址。
sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
//recvfrom函数用于接收数据，同时可获取发送方的地址信息。
recvfrom(sockfd, recvline, sizeof(recvline), 0, (struct sockaddr *)&cliaddr, &cliaddr_len);

```
对于数据报协议，recvfrom返回0是可以 接受的。既然udp是无连接的也不必关闭套接字。
### 8.2 UDP 相关的问题及解决思路

    数据丢失：由于 UDP 不保证数据的可靠传输，可能会出现数据丢失的情况。可通过应用层的确认机制、重传机制来解决，例如客户端发送数据后等待服务器的确认信息，如果超时未收到确认则重传数据。
    数据乱序：UDP 不保证数据的顺序性，接收方可能会收到乱序的数据。可在应用层为每个数据包添加序号，接收方根据序号对数据进行排序。
    UDP 分片：当 UDP 数据报的长度超过网络的最大传输单元（MTU）时，会进行分片。接收方需要对分片的数据进行重组。可使用 IP 首部的相关字段（如标识、标志位、片偏移）来实现数据的重组。

大多数tcp服务器是并发的，而大多数udp是迭代的
每个udp套接字都 有一个接收缓冲区，大小是有限的。
如果服务器未运行，客户端 发送数据报，还是可能先发送arp请求，然后收到一个icmp端口不可达消息。
### 8.3 异步错误
异步错误是指在套接字上发生的错误，而不是在套接字上执行的操作。
**connect**
- 除非套接字已连接，否则异步错误 是不会返回给udp套接字进程的。
    - 如果udp采用connect,不使用sendto和recvfrom,必须使用write和send 
    - 如果udp采用connect,不使用recvfrom,而使用read,recv或者 recvmsg 
- 客户和服务器都 可能调用connect
udp 发送端淹没接收端是轻而易举之事。

## 第九章 基本sctp
### 9.1 sctp协议概述

    多流特性：SCTP 允许在一个连接中存在多个独立的逻辑流，不同流之间的数据可以并行传输，且一个流中的数据丢失不会影响其他流的数据传输，提高了传输效率和可靠性。例如，在多媒体应用中，可以将音频和视频数据分别放在不同的流中传输。
    多宿特性：一个 SCTP 端点可以绑定多个 IP 地址，当其中一个网络接口出现故障时，连接可以快速切换到其他可用的接口，增强了连接的健壮性和容错能力，适用于对网络可靠性要求较高的场景，如电信网络。
    消息边界保留：SCTP 能够保留应用层消息的边界，发送的消息会完整地被接收，不会像 TCP 那样出现粘包问题，方便应用层处理数据。

与 TCP、UDP 对比

    与 TCP 相比，SCTP 提供了多流和多宿功能，而 TCP 是单流、单宿的；TCP 主要保证可靠的字节流传输，SCTP 在此基础上保留消息边界。
    与 UDP 相比，UDP 是无连接、不可靠的，不保证消息的顺序和完整性，而 SCTP 是面向连接、可靠的，提供了可靠的数据传输机制。

### 9.2 sctp套接字编程

        1.使用socket函数创建 SCTP 套接字，协议族为AF_INET（IPv4）或AF_INET6（IPv6），套接字类型为SOCK_SEQPACKET，协议为IPPROTO_SCTP。 
```c
int sockfd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
if (sockfd < 0) {
    perror("socket creation failed");
    return -1;
}
```
    2. 绑定地址和端口
        使用bind函数将 SCTP 套接字绑定到指定的地址和端口，示例：
    地址结构 主要使用struct sockaddr_in（IPv4）或struct sockaddr_in6（IPv6）来存储对端地址信息，与 TCP 和 UDP 类似，需要设置地址族、端口号和 IP 地址等信息
```c

struct sockaddr_in servaddr;
memset(&servaddr, 0, sizeof(servaddr));
servaddr.sin_family = AF_INET;
servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
servaddr.sin_port = htons(1234);
if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    close(sockfd);
    return -1;
}
```
    3.监听连接
        使用listen函数将套接字设置为监听状态，准备接受客户端的连接请求，与 TCP 类似：
```c

if (listen(sockfd, 5) < 0) {
    perror("listen failed");
    close(sockfd);
    return -1;
}
```
    4.接受连接
        使用sctp_accept（或accept，某些实现中）函数接受客户端的连接，返回一个新的套接字用于与客户端通信。
```c

int connfd = sctp_accept(sockfd, NULL, 0);
if (connfd < 0) {
    perror("accept failed");
    close(sockfd);
    return -1;
}
```
    5.数据接收与发送
        使用sctp_recvmsg函数接收数据，可以获取消息的各种信息，如流编号等；使用sctp_sendmsg函数发送数据，可以指定流编号等参数。
```c

// 接收数据
struct sctp_sndrcvinfo sri;
memset(&sri, 0, sizeof(sri));
int n = sctp_recvmsg(connfd, buffer, sizeof(buffer), (struct sockaddr *)&cliaddr, &clilen, &sri, 0);
if (n < 0) {
    perror("sctp_recvmsg failed");
    close(connfd);
    return -1;
}

// 发送数据
sri.sinfo_stream = 1;  // 指定流编号
n = sctp_sendmsg(connfd, sendbuf, strlen(sendbuf), (struct sockaddr *)&cliaddr, clilen, 0, 0, sri.sinfo_stream, 0, 0);
if (n < 0) {
    perror("sctp_sendmsg failed");
    close(connfd);
    return -1;
}
```

## 第十章 sctp 客户/服务器例子

### 10.1 SCTP 服务端程序
基本流程

    套接字创建：使用socket函数创建 SCTP 套接字，指定协议族为AF_INET（IPv4）或AF_INET6（IPv6），套接字类型为SOCK_SEQPACKET，协议为IPPROTO_SCTP。
    地址绑定：使用bind函数将套接字与特定的 IP 地址和端口绑定，使得服务器可以在该地址和端口上监听客户端连接。
    监听连接：使用listen函数将套接字设置为监听状态，准备接受客户端的连接请求。
    接受连接：使用accept函数接受客户端的连接，返回一个新的套接字用于与客户端进行通信。
    数据处理：使用sctp_recvmsg函数接收客户端发送的数据，使用sctp_sendmsg函数向客户端发送响应数据。

代码示例关键部分

```c
// 创建套接字
sockfd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
// 绑定地址
bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
// 监听连接
listen(sockfd, LISTENQ);
// 接受连接
connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen);
// 接收数据
n = sctp_recvmsg(connfd, buffer, BUFFER_SIZE, NULL, 0, &sri, &msg_flags);
// 发送数据
sctp_sendmsg(connfd, response, strlen(response), NULL, 0, 0, 0, sri.sinfo_stream, 0, 0);
```
注意事项

    错误处理：在每个关键系统调用（如socket、bind、listen、accept等）后，都需要检查返回值，若出现错误应进行相应处理，如输出错误信息、关闭套接字等。
    资源管理：在程序结束时，需要关闭所有打开的套接字，避免资源泄漏。

### 10.2 SCTP 客户端程序
基本流程

    套接字创建：同样使用socket函数创建 SCTP 套接字。
    连接服务器：使用sctp_connectx（或connect，某些实现中）函数连接到服务器，指定服务器的地址信息。
    数据发送和接收：使用sctp_sendmsg函数向服务器发送数据，使用sctp_recvmsg函数接收服务器的响应数据。

代码示例关键部分

```c
// 创建套接字
sockfd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
// 连接服务器
sctp_connectx(sockfd, (struct sockaddr *)&servaddr, 1, NULL);
// 发送数据
sctp_sendmsg(sockfd, request, strlen(request), NULL, 0, 0, 0, 0, 0, 0);
// 接收数据
n = sctp_recvmsg(sockfd, buffer, BUFFER_SIZE, NULL, 0, &sri, &msg_flags);
```
注意事项

    服务器地址配置：确保客户端配置的服务器地址和端口与服务器实际监听的地址和端口一致。
    数据格式：客户端和服务器需要协商好数据的格式和协议，以便正确解析和处理数据。

### 10.3 多流
多流特性概述

SCTP 的多流特性允许在一个 SCTP 连接中同时存在多个独立的逻辑流。不同流之间的数据可以并行传输，一个流中的数据丢失或延迟不会影响其他流的数据传输。
多流的使用

    流编号：在发送和接收数据时，可以通过sctp_sndrcvinfo结构体中的sinfo_stream成员指定流编号，从而将数据发送到特定的流中。
    提高性能：对于需要同时传输多种类型数据的应用场景，如多媒体应用中的音频和视频流，可以将不同类型的数据分别放在不同的流中传输，提高传输效率。

代码示例

```c
// 设置流编号
sri.sinfo_stream = 1;
// 发送数据到指定流
sctp_sendmsg(sockfd, data, strlen(data), NULL, 0, 0, 0, sri.sinfo_stream, 0, 0);
```
### 10.4 探究头端阻塞
头端阻塞概念

头端阻塞（Head-of-Line Blocking）是指在一个流中，前面的数据包丢失或延迟会导致后续数据包的处理被阻塞，即使其他流中的数据可以正常传输。
SCTP 中的头端阻塞

虽然 SCTP 的多流特性可以在一定程度上缓解头端阻塞问题，但在某些情况下，头端阻塞仍然可能发生。例如，当一个流中的数据包丢失，接收方需要等待该数据包重传，这可能会影响该流后续数据包的处理。
解决方法

    调整流的优先级：可以通过设置不同流的优先级，让重要的流优先处理，减少头端阻塞对关键数据的影响。
    合理规划流的使用：根据应用的需求，合理分配数据到不同的流中，避免某个流出现长时间的阻塞。


通过本章的学习，可以深入了解 SCTP 在客户 / 服务器编程中的应用，掌握 SCTP 的多流特性以及如何处理头端阻塞问题，为开发高效、可靠的网络应用提供基础。

## 第11章 名字与地址转换 

本章主要讲解域名系统（DNS）及其相关函数，用于在主机名、服务名和IP地址之间进行转换。内容涵盖了IPv4和IPv6环境下的地址转换函数，以及如何编写可重入的代码。

### 1. 域名系统（DNS）

- **DNS的作用**：DNS 是将人类可读的主机名（如 `www.example.com`）转换为机器可读的 IP 地址（如 `192.0.2.1`）的分布式数据库系统。
- **DNS查询过程**：应用程序通过调用特定的函数（如 `gethostbyname`）向 DNS 服务器发起查询，DNS 服务器返回对应的 IP 地址或其他信息。

### 2. 主要函数

#### 2.1 主机名与IP地址转换

- **`gethostbyname`**：
  - 功能：根据主机名获取主机信息（IPv4）。
  - 返回值：指向 `hostent` 结构体的指针，包含主机名、别名列表、地址类型、地址长度和地址列表。
  - 缺点：不支持 IPv6，且不是线程安全的。

- **`gethostbyaddr`**：
  - 功能：根据 IP 地址获取主机信息（IPv4）。
  - 返回值：指向 `hostent` 结构体的指针。
  - 缺点：不支持 IPv6，且不是线程安全的。

- **`getaddrinfo`**：
  - 功能：根据主机名和服务名获取地址信息（支持 IPv4 和 IPv6）。
  - 返回值：指向 `addrinfo` 结构体链表的指针，包含地址族、套接字类型、协议、地址和端口等信息。
  - 优点：支持 IPv6，线程安全。
  - 相关函数：
    - **`freeaddrinfo`**：释放 `getaddrinfo` 返回的 `addrinfo` 结构体链表。
    - **`gai_strerror`**：将 `getaddrinfo` 的错误代码转换为可读的错误信息。

- **`getnameinfo`**：
  - 功能：根据套接字地址获取主机名和服务名（支持 IPv4 和 IPv6）。
  - 返回值：成功返回 0，失败返回错误代码。
  - 优点：支持 IPv6，线程安全。

#### 2.2 服务名与端口号转换

- **`getservbyname`**：
  - 功能：根据服务名获取服务信息。
  - 返回值：指向 `servent` 结构体的指针，包含服务名、别名列表、端口号和协议类型。

- **`getservbyport`**：
  - 功能：根据端口号获取服务信息。
  - 返回值：指向 `servent` 结构体的指针。

- **`htons`**：
  - 功能：将主机字节序转换为网络字节序（16位）。
  - 返回值：转换后的网络字节序值。
- **`ntohs`**：
  - 功能：将网络字节序转换为主机字节序（16位）。
  - 返回值：转换后的主机字节序值。
  
#### 2.3 可重入函数

- **可重入性**：可重入函数可以在多个线程中同时调用，而不会导致数据竞争或不一致。
- **`gethostbyname_r`** 和 **`gethostbyaddr_r`**：
  - 功能：分别是 `gethostbyname` 和 `gethostbyaddr` 的可重入版本。
  - 参数：增加了缓冲区指针和错误指针，用于存储结果和错误信息。
  - 优点：线程安全。

### 3. 实用函数

- **`host_serv`**：
  - 功能：封装 `getaddrinfo`，简化主机名和服务名的解析。
  - 返回值：指向 `addrinfo` 结构体的指针。

- **`tcp_connect`**：
  - 功能：建立 TCP 连接。
  - 返回值：成功返回套接字描述符，失败返回 -1。

- **`tcp_listen`**：
  - 功能：创建 TCP 监听套接字。
  - 返回值：成功返回套接字描述符，失败返回 -1。

- **`udp_client`**：
  - 功能：创建 UDP 客户端套接字。
  - 返回值：成功返回套接字描述符，失败返回 -1。

- **`udp_connect`**：
  - 功能：建立 UDP 连接。
  - 返回值：成功返回套接字描述符，失败返回 -1。

- **`udp_server`**：
  - 功能：创建 UDP 服务器套接字。
  - 返回值：成功返回套接字描述符，失败返回 -1。

### 4. 知识脉络

1. **DNS基础**：理解 DNS 的作用和查询过程。
2. **主机名与IP地址转换**：
   - 掌握 `gethostbyname`、`gethostbyaddr`、`getaddrinfo` 和 `getnameinfo` 的使用方法和区别。
   - 理解 `getaddrinfo` 的优势（支持 IPv6 和线程安全）。
3. **服务名与端口号转换**：
   - 掌握 `getservbyname` 和 `getservbyport` 的使用方法。
4. **可重入函数**：
   - 理解可重入性的概念。
   - 掌握 `gethostbyname_r` 和 `gethostbyaddr_r` 的使用方法。
5. **实用函数**：
   - 掌握 `host_serv`、`tcp_connect`、`tcp_listen`、`udp_client`、`udp_connect` 和 `udp_server` 的使用方法。

### 5. 复习要点

- **重点函数**：`getaddrinfo`、`getnameinfo`、`gethostbyname_r`、`gethostbyaddr_r`。
- **IPv6支持**：理解如何在 IPv6 环境下使用这些函数。
- **线程安全**：理解可重入函数的重要性及其实现方式。
- **实用函数**：掌握常用网络编程函数的封装和使用。

## 第12章 IPv4 和 IPv6 的互操作性 

本章主要讨论 IPv4 和 IPv6 的互操作性问题，即在 IPv4 和 IPv6 共存的环境中，如何实现两者之间的通信。内容涵盖了双栈主机、地址转换、协议兼容性以及相关的编程技术。

---

### 1. IPv4 和 IPv6 的共存

- **背景**：
  - IPv4 和 IPv6 是两种不同的网络层协议，IPv6 是 IPv4 的下一代协议，旨在解决 IPv4 地址耗尽等问题。
  - 在 IPv6 逐步取代 IPv4 的过程中，两者会长期共存，因此需要解决它们之间的互操作性问题。

- **双栈主机（Dual-Stack Host）**：
  - 双栈主机同时支持 IPv4 和 IPv6 协议栈，能够处理两种协议的通信。
  - 双栈主机可以通过 IPv4 或 IPv6 与其他主机通信，具体取决于对方的协议支持情况。

---

### 2. IPv4 和 IPv6 的地址转换

- **IPv4 映射的 IPv6 地址（IPv4-Mapped IPv6 Address）**：
  - 格式：`::ffff:IPv4-address`，例如 `::ffff:192.0.2.1`。
  - 作用：用于在 IPv6 环境中表示 IPv4 地址，使得 IPv6 应用程序能够与 IPv4 主机通信。
  - 使用场景：当 IPv6 应用程序与 IPv4 主机通信时，操作系统会自动将 IPv4 地址转换为 IPv4 映射的 IPv6 地址。

- **IPv4 兼容的 IPv6 地址（IPv4-Compatible IPv6 Address）**：
  - 格式：`::IPv4-address`，例如 `::192.0.2.1`。
  - 作用：用于在 IPv6 环境中表示 IPv4 地址，但这种格式已被弃用，推荐使用 IPv4 映射的 IPv6 地址。

---

### 3. 协议兼容性

- **套接字选项**：
  - **`IPV6_V6ONLY`**：
    - 作用：控制 IPv6 套接字是否仅支持 IPv6 通信。
    - 默认值：通常为 0（关闭），即允许 IPv6 套接字同时支持 IPv4 和 IPv6 通信。
    - 设置为 1 时，IPv6 套接字仅支持 IPv6 通信。

- **地址族兼容性**：
  - 在双栈主机上，IPv6 套接字可以接受来自 IPv4 和 IPv6 客户端的连接。
  - 当 IPv6 套接字接受 IPv4 客户端的连接时，操作系统会自动将 IPv4 地址转换为 IPv4 映射的 IPv6 地址。

---

### 4. 编程技术

- **`getaddrinfo` 的使用**：
  - `getaddrinfo` 函数支持 IPv4 和 IPv6，能够根据主机名和服务名返回适合的地址列表。
  - 通过设置 `ai_family` 参数为 `AF_UNSPEC`，可以同时获取 IPv4 和 IPv6 地址。

- **`inet_pton` 和 `inet_ntop`**：
  - 用于在文本格式和二进制格式之间转换 IPv4 和 IPv6 地址。
  - `inet_pton`：将文本格式的地址转换为二进制格式。
  - `inet_ntop`：将二进制格式的地址转换为文本格式。

- **地址族判断**：
  - 在编程中，可以通过检查地址族（`sa_family`）来判断地址是 IPv4 还是 IPv6。
  - 例如：
    ```c
    if (addr->sa_family == AF_INET) {
        // IPv4 地址
    } else if (addr->sa_family == AF_INET6) {
        // IPv6 地址
    }
    ```

---

### 5. 互操作性场景

- **IPv6 客户端与 IPv4 服务器通信**：
  - 如果服务器仅支持 IPv4，客户端使用 IPv6 套接字时，操作系统会自动将 IPv4 地址转换为 IPv4 映射的 IPv6 地址。
  - 客户端无需额外处理，可以直接与 IPv4 服务器通信。

- **IPv4 客户端与 IPv6 服务器通信**：
  - 如果服务器支持双栈，IPv4 客户端可以直接与 IPv6 服务器通信。
  - 服务器接收到 IPv4 客户端的连接时，操作系统会将 IPv4 地址转换为 IPv4 映射的 IPv6 地址。

- **双栈主机的通信**：
  - 双栈主机可以根据对方的协议支持情况，选择使用 IPv4 或 IPv6 进行通信。
  - 优先使用 IPv6（如果双方都支持），否则回退到 IPv4。

---

### 6. 知识脉络

1. **双栈主机**：
   - 理解双栈主机的概念及其在 IPv4 和 IPv6 共存环境中的作用。
2. **地址转换**：
   - 掌握 IPv4 映射的 IPv6 地址和 IPv4 兼容的 IPv6 地址的格式及用途。
3. **协议兼容性**：
   - 理解 `IPV6_V6ONLY` 套接字选项的作用及其对通信的影响。
4. **编程技术**：
   - 掌握 `getaddrinfo`、`inet_pton` 和 `inet_ntop` 的使用方法。
   - 学会判断地址族并处理 IPv4 和 IPv6 地址。
5. **互操作性场景**：
   - 理解 IPv6 客户端与 IPv4 服务器、IPv4 客户端与 IPv6 服务器的通信机制。

---

### 7. 复习要点

- **双栈主机**：理解双栈主机的概念及其在 IPv4 和 IPv6 共存环境中的作用。
- **地址转换**：掌握 IPv4 映射的 IPv6 地址的格式及用途。
- **套接字选项**：理解 `IPV6_V6ONLY` 的作用及其对通信的影响。
- **编程函数**：熟练掌握 `getaddrinfo`、`inet_pton` 和 `inet_ntop` 的使用方法。
- **互操作性场景**：理解 IPv4 和 IPv6 之间的通信机制。

## 第十三 守护进程 

### 一、守护进程启动方式
1. **系统初始化脚本启动**
    - **启动位置**：在系统启动阶段，由位于 `/etc` 目录或 `/etc/rc` 开头目录的系统初始化脚本启动。
    - **特权与示例**：此类守护进程一开始就拥有超级用户特权，像 `inetd`、Web 服务器、邮件服务器、`syslogd` 等都通过这种方式启动。它们为系统提供基础服务，在系统启动时就被加载以确保服务的持续性。
2. **inetd 超级服务器启动**
    - **启动关系**：`inetd` 由系统初始化脚本启动，它监听各种网络请求。当接收到请求时，`inetd` 会启动实际处理该请求的服务器。
    - **适用服务**：常见的如 Telnet、FTP 服务器等网络服务由 `inetd` 启动。这种方式优化了资源利用，避免了每个服务都常驻内存带来的资源浪费。
3. **cron 定期启动**
    - **启动机制**：`cron` 按预设规则定期执行一些程序，这些程序通常是守护进程。`cron` 自身由系统初始化脚本启动。
    - **应用场景**：常用于执行周期性任务，如定期备份、日志清理等，确保系统的维护和管理自动化。
4. **at 指令特定时刻启动**
    - **启动特点**：`at` 指令用于指定将来某个时刻执行某个程序，由 `at` 启动的程序也常为守护进程。
    - **使用场景**：适用于一次性的、在特定时间执行的任务，比如在凌晨系统负载较低时进行数据处理等操作。
5. **用户终端启动（测试用途）**
    - **启动方式**：守护进程还能从用户终端以前台或后台方式启动，这种方式主要用于测试程序。
    - **作用**：方便开发人员在开发和调试守护进程时，快速启动并观察其运行状态和输出，以便及时发现和解决问题。

### 二、syslogd 守护进程
1. **syslogd 守护进程概述**
    - **功能**：守护进程没有控制终端，不能使用 `printf` 等面向终端的输出函数。`syslogd` 守护进程用于接收和处理系统中各种守护进程发送的日志信息。
2. **syslogd 启动步骤**
    - **读取配置文件**：启动时，`syslogd` 首先读取配置文件，通常为 `/etc/syslog.conf`。该配置文件指定了本进程可能收取的日志信息应如何处理，例如将不同级别的日志信息发送到不同的文件或设备。
    - **创建 Unix 域数据报套接字**：`syslogd` 创建一个 Unix 域数据报套接字，并捆绑路径名 `/var/run/log` 或 `/dev/log`。这个套接字用于接收来自本地系统其他进程发送的日志信息，提供了一种高效的本地进程间通信方式。
    - **创建 UDP 套接字**：同时，`syslogd` 创建一个 UDP 套接字，并捆绑端口号 514，该端口是 syslog 使用的标准端口。这使得远程系统也能通过网络将日志信息发送给 `syslogd`，实现分布式系统的日志集中管理。
    - **打开内核日志设备**：`syslogd` 打开路径名 `/dev/klog`，将来自内核中的任何出错消息视为该设备的输入。这样，内核产生的日志信息也能被 `syslogd` 收集和处理。
    - **运行循环**：此后，`syslogd` 守护进程在一个无限循环中运行。它调用 `select` 函数等待上述三个描述符（Unix 域套接字、UDP 套接字、`/dev/klog` 对应的描述符）之一变为可读状态。一旦有描述符可读，就从中读入日志消息。如果收到 `SIGHUP` 事件，`syslogd` 会重新读取配置文件，以便根据新的配置处理日志信息。

### 三、syslog 函数
1. **功能**：`syslog` 函数是守护进程用于输出信息的函数，它将信息发送给 `syslogd` 守护进程。通过这个函数，守护进程能够将运行过程中的各种信息（如错误信息、状态信息等）发送到统一的日志管理中心，方便系统管理员进行监控和故障排查。
2. **使用**：`syslog` 函数可以根据不同的日志级别（如 `LOG_EMERG`、`LOG_ALERT`、`LOG_CRIT`、`LOG_ERR`、`LOG_WARNING`、`LOG_NOTICE`、`LOG_INFO`、`LOG_DEBUG`）对日志信息进行分类，使得管理员能够根据日志级别快速定位重要问题。

### 四、daemon_init 函数
1. **功能**：`daemon_init` 函数用于初始化守护进程，通常包含创建守护进程的一系列标准步骤，如创建子进程、脱离控制终端、创建新会话、关闭不必要文件描述符等操作，确保守护进程能够在后台独立、稳定地运行。
2. **实现要点**：在实现 `daemon_init` 函数时，需严格按照守护进程创建的规范流程进行，并且要注意错误处理，确保在初始化过程中出现问题时能够进行适当的处理，避免守护进程异常终止。

```c
void daemonize(const char *path) {
    // 第一次 fork, 让父进程退出,目的是让子进程在后台运行，脱离终端
    pid_t pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); // 父进程退出
    }
    //目前为止，子进程已经是会话的领头进程，脱离了终端，但它仍然继承了终端、进程组和会话

    // 第一次fork后，创建新会话，成为会话的领头进程，脱离终端和进程组
    //会话领导者是一个新会话的起点，跟过去的终端没有关系了
    //如果不做这一步，守护进程仍然可以打开终端，并且重新获得终端，此时关闭终端 会使应用程序崩溃（收到SIGHUP信号）
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }
    //目前为止，子进程不再有控制终端，但它仍然可以打开终端，并且仍然可以重新获得终端

    // 第二次 fork，让父进程退出，目的是彻底脱离终端，彻底成为一个守护进程
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); // 第一次 fork 的子进程退出
    }
    // 至此，守护进程彻底脱离了终端、会话和进程组,成为一个完全独立的进程。也不是会话的领头进程了。  
    // 关闭文件描述符
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // 重定向标准输入、输出、错误到 /dev/null
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);

    openlog(path, LOG_PID, 0);
}
```

### 五、inetd 守护进程
1. **功能与特点**：`inetd` 作为 Internet 超级服务器，监听多个网络服务端口。它的主要特点是资源优化，通过统一监听并按需启动实际服务进程，减少了系统中常驻进程的数量，提高了系统资源利用率。
2. **工作流程**：`inetd` 启动后，读取配置文件（如 `/etc/inetd.conf`），根据配置监听相应端口。当有客户端连接请求到达时，`inetd` 根据请求的端口号找到对应的服务程序，使用 `exec` 系列函数启动该服务程序，并将客户端的连接套接字传递给新启动的服务程序，由其负责与客户端进行通信。

### 六、如何在系统启动时能启动一个用户 定义 的程序？
1. **systemd** 
    在/etc/systemd/system/下面建一个新的用户 服务 文件 。
    例如 /etc/systemd/system/clash.service
    Type=forking：表明服务以分叉（fork）的方式启动，适合使用 nohup 和 & 启动的程序。
```ini
[Unit]
Description=Clash Daemon Service
After=network.target

[Service]
Type=forking 
ExecStart=/home/expert/clash/start.sh
ExecStop=/bin/kill -s SIGTERM $MAINPID
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```
sudo systemctl daemon-reload
sudo systemctl enable clash.service
2. **/etc/rc.local（传统方法）**

    在 /etc/rc.local 文件中添加启动脚本。
3. **使用 crontab 的 @reboot**

    在用户的 crontab 文件中添加一行，指定在系统启动时运行脚本。
    例如：
    ```bash
    @reboot /path/to/your/script
    ```
4. **.desktop**
    ~/.config/autostart/ 目录下创建一个 .desktop 文件
    ```ini
    [Desktop Entry]
    Type=Application
    Exec=/path/to/your/program
    Hidden=false
    NoDisplay=false
    X-GNOME-Autostart-enabled=true
    Name=My Custom Program
    Comment=Start My Custom Program at login
    ```
## 第十四 高级IO函数 
高级I/O函数与基本I/O函数（read/write）的区别主要体现在以下几个方面：
1. 阻塞与非阻塞：
    - read/write 是阻塞的，即调用后进程会一直阻塞直到数据可用或发生错误。
    - 高级I/O函数（如 recv、send、recvfrom、sendto）提供了非阻塞选项，允许进程在等待数据时不阻塞。
2. IO多路复用：
    - 高级I/O函数（如 select、poll、epoll）提供了一种高效的IO多路复用机制，允许进程同时监视多个文件描述符的状态变化，从而实现高效的事件驱动编程。
3. 记录锁与文件 锁：
    - 高级I/O函数（如 flock、fcntl）提供了记录锁和文件锁机制，用于控制对文件的访问权限，防止多个进程同时修改文件。
4. 内存映射：
    - 高级I/O函数（如 mmap）提供了内存映射机制，允许进程将文件映射到内存中，实现高效的文件读写操作。
5. 超时机制：
    - 高级I/O函数（如 recvfrom、sendto）提供了超时机制，允许进程在等待数据时设置超时时间。
6. 信号驱动：
    - 高级I/O函数（如 sigaction）提供了信号驱动的I/O机制，允许进程在等待数据时注册信号处理函数，当数据可用时触发信号。
7. 异步I/O：
    - 高级I/O函数（如 aio_read、aio_write）提供了异步I/O机制，允许进程在等待数据时不阻塞，而是在数据可用时通过回调函数通知进程。
8. 分散聚集：
    - 高级I/O函数（如 readv、writev）提供了分散和聚集的I/O机制，允许进程一次性读取或写入多个分散的内存块。

特性	|基本 I/O 函数（read/write）	|高级 I/O 函数
---|---|---
阻塞/非阻塞	|默认阻塞	|支持非阻塞
I/O 多路复用|	不支持	|支持（select、poll、epoll）
记录锁/文件锁|	不支持	|支持（fcntl、flock）
内存映射|	不支持	|支持（mmap）
超时机制|	不支持	|支持
信号驱动 I/O|	不支持	|支持
分散/聚集 I/O|	不支持	|支持（readv/writev）
性能与灵活性|	较低	|较高

### 14.1. 套接字超时

    核心问题：在网络编程中，I/O操作可能会阻塞，需要设置超时机制以避免无限等待。
    设置超时的方法：

#### alarm 信号：使用 alarm 函数设置定时器，超时后触发 SIGALRM 信号。
        代码例子  timeout.c
        使用SIGALRM 为recvfrom设置超时，为connect设置超时，
#### select：select 函数允许进程监视多个文件描述符的读、写或异常状态变化，其第三个参数为超时时间。
        通过将 I/O 操作与 select 结合，可在等待 I/O 事件发生时设置超时。
        代码例子：使用select为recvfrom设置超时

#### setsockopt：通过 SO_RCVTIMEO 和 SO_SNDTIMEO 选项设置套接字的接收和发送超时。
        例子：使用SO_RCVTIMEO 为recvfrom设置超时

        缺点：并非所有系统都支持，且不适用于 connect 函数。

    注意事项：

        alarm 可能会干扰其他信号处理逻辑。

        select 适用于多路复用场景，但效率较低。

        setsockopt 是最直接的方法，但并非所有系统都支持
    connect 函数的超时设置：通过将套接字设置为非阻塞模式，结合 select 函数实现 connect 超时。

### 14.2 read和write变体
- recv和send函数 
与 read 和 write 类似，但可通过第四个参数传递标志位，实现更灵活的数据收发控制。
- readv 和sendv函数
readv 实现分散读，将数据从文件描述符读到多个分散的内存块中；writev 实现集中写，将多个分散的内存块数据一并写入文件描述符。
- recvmsg 和sendmsg函数 
功能最强大，可实现基本数据收发、接收和发送辅助数据（如控制信息），结合了其他 I/O 函数特性。
作者表达 了这2个函数是最通用的函数，意指我们尽量使用它们。

### 14.3 高级IO函数
- **dup 和 dup2**：
 dup
功能：创建一个新的文件描述符，新描述符与原描述符指向相同的文件、管道或套接字等，新描述符是当前可用的最小整数值。
dup2
功能：与 dup 类似，但可指定新描述符的值。如果已指定的描述符已经打开 则可以 先关闭它。
- **fcntl**：
fcntl
功能：对文件描述符进行各种控制操作，如设置文件状态标志、获取/设置文件描述符标志、获取/设置异步 I/O 通知等。
- **ioctl**：
ioctl
功能：对设备进行各种控制操作，如设置套接字选项、获取/设置网络接口参数等。
- **pselect 和 ppoll**：
pselect 和 ppoll            
- **sendfile**:
功能：在两个文件描述符之间直接传递数据（内核中进行），避免用户空间和内核空间的数据拷贝，实现高效零拷贝，常用于文件传输场景。
- **mmap**:
功能：用于申请一段内存，可将文件映射到内存中，实现内存与文件的关联，方便对文件进行操作，也可用于进程间共享内存。
- **splice**:
功能：在两个文件描述符之间直接传递数据（内核中进行），避免用户空间和内核空间的数据拷贝，实现高效零拷贝，常用于文件传输场景。
- **tee**:
功能：将一个文件描述符的数据同时复制到两个文件描述符，常用于日志记录和数据备份等场景。

## 第十五 域协议 
### 15.1 域套接字概述
1.特点
- **高性能：**
- **本地通信:**
- **与套接字接口相似:**
2.应用场景 
- **本地进程通信**
- **替代管道和消息队列**

域协议 并不是一个完整的协议族，而是在单个主机上执行客户服务器通信 的一种方法，但所用的api是套接字api.
unix 域协议提供 两类套接字：
- 流套接字：提供有序、可靠、双向字节流服务的套接字。
- 数据报套接字：提供数据报服务的套接字。

### 15.2 域套接字地址结构
- **sockaddr_un**：
    - 功能：用于本地域套接字地址的结构体。
    - 成员：
        - sun_family：地址族，固定为 AF_UNIX。
        - sun_path：套接字文件路径，以 null 结尾的字符串。
- **bind**：
    - 功能：将套接字与本地域套接字地址绑定。
    - 参数：
        - sockfd：套接字描述符。
        - addr：指向 sockaddr_un 结构体的指针。路径名也是有要求的，必须是存在 的路径 ，不能是文件 。不能超过UNIX_PATH_MAX
        - addrlen：sockaddr_un 结构体的长度。
    - 返回值：成功返回 0，失败返回 -1。
- **socketpair**：
    - 功能：创建一对[未命名]的本地域套接字。
    - 参数：
        - domain：地址族，固定为 AF_UNIX。
        - type：套接字类型，SOCK_STREAM 或 SOCK_DGRAM。
        - protocol：协议，通常为 0。
        - sockfd[2]:新创建 的2个套接字作为sockfd返回 

### 15.3 字节流套接字
1.创建套接字
```c
#include <sys/socket.h>

int sockfd = socket(AF_UNIX, SOCK_STREAM, 0); // 字节流套接字
int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);  // 数据报套接字
```
2.绑定套接字
```c
#include <sys/socket.h>
#include <sys/un.h>
struct sockaddr_un addr;
addr.sun_family = AF_UNIX;
strcpy(addr.sun_path, "/tmp/mysocket");
int bind_result = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
```
3.连接套接字
```c
#include <sys/socket.h>
#include <sys/un.h>
struct sockaddr_un addr;
addr.sun_family = AF_UNIX;
strcpy(addr.sun_path, "/tmp/mysocket");
int connect_result = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
```
4.监听套接字
```c
#include <sys/socket.h>
int listen_result = listen(sockfd, 5);
```
5.接受连接
```c
#include <sys/socket.h>
int client_sockfd = accept(sockfd, NULL, NULL);
```
6.发送和接收数据
```c
#include <sys/socket.h>
send(sockfd, "Hello", 5, 0);
recv(sockfd, buffer, sizeof(buffer), 0);
```
7.关闭套接字
```c
#include <unistd.h>
close(sockfd);
```
### 15.4 unix数据报套接字
1.特点：无连接 ，不可靠，有最大长度 限制


## 第十六 非阻塞 io

### 16.1 套接字默认是阻塞的
套接字的默认状态是阻塞的，这就意味着发出一个不能立即完成 的套接字调用时，其进程将被投入睡眠，等待相应操作完成 。可能阻塞的套接字调用分为四类：
- 1. 输入操作：包括 read、readv、recv、recvfrom 和 recvmsg。接收缓冲没有数据则进程进入休眠。
对于非阻塞的来说，如果输入操作不能被满足，套接字返回 一个EWOULDBLOCK 错误。
- 2. 输出操作：包括 write、writev、send、sendto 和 sendmsg。发送缓冲区没有空间，进程进入休眠。
对于非阻塞的来说，如果发送没有空间，套接字返回EWOULDBLOCK错误。
- 3. 接受连接操作： accept。对于一个阻塞的套接字调用accept，新的连接到来之前处于休眠。
对于非阻塞的来说，如果新的连接尚未到达 ，立即返回一个EWOULDBLOCK
- 4. 发出连接，即connect；由于TCP的三次握手，connect一直等到客户收到自己的SYN的ACK才返回。这意味着connect至少阻塞进度一个RTT时间。
对于非阻塞的来说，调用connect并且连接不能建立，三次握手照样发起。不过会返回一个EINPROGRESS错误。
对于不能满足的非阻塞IO操作，返回EAGAIN错误。
等待操作：包括 select 和 poll。

### 16.2 非阻塞读和写
非阻塞套接字调用：
- 对于输入操作，当套接字接收缓冲区中没有数据时，recv 调用将返回 0，而不是阻塞进程。
- 对于输出操作，当套接字发送缓冲区已满时，send 调用将返回 -1，而不是阻塞进程。
- 对于连接操作，当套接字处于非阻塞模式时，connect 调用将立即返回，而不会阻塞进程。
- 对于等待操作，当没有描述符准备好时，select 和 poll 调用将立即返回，而不会阻塞进程。
非阻塞套接字的设置：
- 对于套接字的设置，可以使用 fcntl 函数的 F_SETFL 命令。
- 对于套接字的设置，可以使用 ioctl 函数的 FIONBIO 命令。
- 对于套接字的设置，可以使用 SO_NONBLOCK 选项。
- 对于套接字的设置，可以使用 O_NONBLOCK 标志。
非阻塞套接字的使用：
- 对于非阻塞套接字的使用，需要在套接字创建后立即设置为非阻塞模式。
- 对于非阻塞套接字的使用，需要在套接字创建后立即设置为非阻塞模式。

《unix网络编程卷1：套接字api》第3版第16章说道：当有一个已经完成的连接准备好被accept时，select作为可读描述符返回该连接的监听套接字。因此，如果我们使用select 作为某个监听套接字上等待一个外来连接，那就没有必要把该监听套接字设置为非阻塞。这是因为如果select告诉我们该套接字已经有连接就绪，那么随后 的accept调用就不应该阻塞。 
- select 作用： select 监听套接字，在完成队列 不为空时将其标记为可读，并返回。
- accept 作用： 当select告诉监听可读时，accept一定会返回一个新的连接套接字（而不会阻塞）。

但这里存在一个时间陷阱问题，如果select告诉我们有一个连接就绪，但是accept调用之前可能队列中的连接被TCP驱逐出队或者被其它线程接受，accept调用会阻塞，那么select会一直阻塞，直到accept返回。

(1) 解决办法是将监听套接字设置为非阻塞，这样select就不会阻塞，accept调用也不会阻塞。如果 accept 返回 EWOULDBLOCK 或 EAGAIN，我们可以简单地忽略这个错误，并继续调用 select 等待新的连接。
(2)  使用边缘触发（Edge-Triggered）模式.在边缘触发模式下，epoll 只会在状态发生变化时通知你一次。因此，你必须在一次通知中处理所有已完成的连接，直到 accept 返回 EWOULDBLOCK 或 EAGAIN。
(3) 使用线程池或多进程 在多线程或多进程服务器中，可以让多个线程或进程同时调用 accept。操作系统会确保每个连接只被一个线程或进程接受。

## 第十七 ioctl
《unix网络编程卷1：套接字api》第3版第17章主要介绍 ioctl函数 ，请帮我总结这一章的复习大纲，供我复习掌握知识。重要的知识点或者难以理解的地方可以提供代码示例。

### 1. ioctl 函数概述
作用：ioctl 是一个通用的设备控制函数，用于对设备或文件描述符进行控制操作。

原型：

```c
 
#include <sys/ioctl.h>
int ioctl(int fd, unsigned long request, ...);
fd：文件描述符（通常是套接字或设备文件）。

request：控制请求代码，指定要执行的操作。

...：可选参数，通常是一个指向数据结构的指针。
```

### 2. ioctl 的常见用途

+ 网络接口控制：

获取或设置网络接口的配置信息（如 IP 地址、子网掩码、MAC 地址等）。

示例：SIOCGIFADDR（获取 IP 地址）、SIOCSIFADDR（设置 IP 地址）。

+ ARP 缓存操作：

添加、删除或查询 ARP 缓存条目。

示例：SIOCSARP（添加 ARP 条目）、SIOCDARP（删除 ARP 条目）。

+ 路由表操作：

查询或修改路由表。

示例：SIOCADDRT（添加路由）、SIOCDELRT（删除路由）。

+ 套接字选项：

获取或设置套接字选项。

示例：FIONBIO（设置非阻塞模式）。

### 3. 重要数据结构
struct ifreq：

用于网络接口的配置和控制。

示例：

```c
Copy
struct ifreq {
    char ifr_name[IFNAMSIZ]; // 接口名称
    union {
        struct sockaddr ifr_addr;    // IP 地址
        struct sockaddr ifr_netmask; // 子网掩码
        struct sockaddr ifr_hwaddr;  // 硬件地址
        // 其他字段
    };
};
struct arpreq：
```
用于 ARP 缓存操作。

示例：

```c
Copy
struct arpreq {
    struct sockaddr arp_pa; // 协议地址（IP 地址）
    struct sockaddr arp_ha; // 硬件地址（MAC 地址）
    int arp_flags;          // 标志（如 ATF_PERM）
};
```
### 4. 常见 ioctl 请求
- 网络接口相关：

SIOCGIFCONF：获取接口配置列表。
SIOCGIFADDR：获取接口的 IP 地址。
SIOCSIFADDR：设置接口的 IP 地址。
SIOCGIFHWADDR：获取接口的硬件地址（MAC 地址）。

- ARP 相关：

SIOCSARP：添加 ARP 缓存条目。

SIOCDARP：删除 ARP 缓存条目。

SIOCGARP：查询 ARP 缓存条目。

- 路由相关：

SIOCADDRT：添加路由条目。

SIOCDELRT：删除路由条目。

- 套接字相关：

FIONBIO：设置非阻塞模式。

### 5. 难点与注意事项
> 权限问题：ioctl 的许多操作需要 root 权限。
> 错误处理：ioctl 的返回值需要仔细检查，错误时设置 errno。
> 数据结构：struct ifreq 和 struct arpreq 的字段需要正确设置。

平台差异：不同操作系统对 ioctl 的支持可能有所不同。
理解 ioctl 的作用和常见用途。
掌握重要数据结构（如 ifreq 和 arpreq）。
熟悉常见 ioctl 请求（如 SIOCGIFADDR、SIOCSARP）。
通过代码示例加深理解，并尝试修改和扩展示例代码

## 第十八 路由套接字

《UNIX 网络编程卷 1：套接字 API》第 3 版第 18 章主要介绍了 **路由套接字**（Routing Socket），这是 UNIX 系统中用于操作路由表的一种高级机制。以下是本章的复习大纲，包括重要知识点和代码示例，帮助你更好地掌握内容。

---

## 第 18 章复习大纲

### 1. **路由套接字概述**
   - **作用**：路由套接字允许用户空间程序与内核路由子系统交互，用于查询、添加、删除或修改路由表。
   - **特点**：
     - 提供了一种灵活的方式来操作路由表。
     - 支持 IPv4 和 IPv6。
     - 使用 `PF_ROUTE` 协议族。

---

### 2. **路由套接字的基本用法**
   - **创建路由套接字**：
     ```c
     int fd = socket(PF_ROUTE, SOCK_RAW, AF_UNSPEC);
     ```
     - `PF_ROUTE`：指定协议族为路由套接字。
     - `SOCK_RAW`：指定套接字类型为原始套接字。
     - `AF_UNSPEC`：指定地址族为未指定（支持 IPv4 和 IPv6）。

   - **发送和接收路由消息**：
     - 使用 `write` 发送路由消息。
     - 使用 `read` 接收路由消息。

---

### 3. **路由消息结构**
   - **`struct rt_msghdr`**：
     - 路由消息的头部结构，包含消息类型、长度、版本等信息。
     - 示例：
       ```c
       struct rt_msghdr {
           u_short rtm_msglen;  // 消息长度
           u_char  rtm_version; // 协议版本
           u_char  rtm_type;    // 消息类型
           u_short rtm_index;   // 接口索引
           int     rtm_flags;   // 标志
           int     rtm_addrs;   // 地址掩码
           pid_t   rtm_pid;     // 进程 ID
           int     rtm_seq;     // 序列号
           int     rtm_errno;   // 错误代码
           int     rtm_use;     // 使用计数
           u_long  rtm_inits;   // 初始化状态
           struct rt_metrics rtm_rmx; // 路由度量
       };
       ```

   - **地址掩码（`rtm_addrs`）**：
     - 用于指定消息中包含的地址类型（如目的地址、网关地址、网络掩码等）。
     - 示例：
       ```c
       #define RTA_DST     0x01  // 目的地址
       #define RTA_GATEWAY 0x02  // 网关地址
       #define RTA_NETMASK 0x04  // 网络掩码
       ```

---

### 4. **常见路由消息类型**
   - **`RTM_ADD`**：添加路由条目。
   - **`RTM_DELETE`**：删除路由条目。
   - **`RTM_GET`**：查询路由条目。
   - **`RTM_CHANGE`**：修改路由条目。
   - **`RTM_REDIRECT`**：重定向路由。
   - **`RTM_MISS`**：路由查找失败。

---

### 5. **代码示例**

#### **(1) 查询路由表**
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/route.h>
#include <net/if.h>
#include <arpa/inet.h>

void query_routing_table() {
    int fd;
    char buf[1024];
    struct rt_msghdr *rtm;
    struct sockaddr_in *sin;

    // 创建路由套接字
    fd = socket(PF_ROUTE, SOCK_RAW, AF_UNSPEC);
    if (fd < 0) {
        perror("socket");
        exit(1);
    }

    // 构造路由查询消息
    memset(buf, 0, sizeof(buf));
    rtm = (struct rt_msghdr *)buf;
    rtm->rtm_msglen = sizeof(struct rt_msghdr);
    rtm->rtm_version = RTM_VERSION;
    rtm->rtm_type = RTM_GET;
    rtm->rtm_addrs = RTA_DST | RTA_GATEWAY | RTA_NETMASK;
    rtm->rtm_pid = getpid();
    rtm->rtm_seq = 1;

    // 发送路由查询消息
    if (write(fd, buf, rtm->rtm_msglen) < 0) {
        perror("write");
        close(fd);
        exit(1);
    }

    // 接收路由查询结果
    while (1) {
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n < 0) {
            perror("read");
            break;
        }

        rtm = (struct rt_msghdr *)buf;
        if (rtm->rtm_type == RTM_GET) {
            sin = (struct sockaddr_in *)(rtm + 1);
            char dst[INET_ADDRSTRLEN], gateway[INET_ADDRSTRLEN], netmask[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &sin->sin_addr, dst, sizeof(dst));
            inet_ntop(AF_INET, &(sin + 1)->sin_addr, gateway, sizeof(gateway));
            inet_ntop(AF_INET, &(sin + 2)->sin_addr, netmask, sizeof(netmask));
            printf("Destination: %s, Gateway: %s, Netmask: %s\n", dst, gateway, netmask);
        }
    }

    close(fd);
}

int main() {
    query_routing_table();
    return 0;
}
```

#### **(2) 添加路由条目**
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/route.h>
#include <net/if.h>
#include <arpa/inet.h>

void add_route(const char *dst, const char *gateway, const char *netmask) {
    int fd;
    char buf[1024];
    struct rt_msghdr *rtm;
    struct sockaddr_in *sin;

    // 创建路由套接字
    fd = socket(PF_ROUTE, SOCK_RAW, AF_UNSPEC);
    if (fd < 0) {
        perror("socket");
        exit(1);
    }

    // 构造路由添加消息
    memset(buf, 0, sizeof(buf));
    rtm = (struct rt_msghdr *)buf;
    rtm->rtm_msglen = sizeof(struct rt_msghdr) + 3 * sizeof(struct sockaddr_in);
    rtm->rtm_version = RTM_VERSION;
    rtm->rtm_type = RTM_ADD;
    rtm->rtm_addrs = RTA_DST | RTA_GATEWAY | RTA_NETMASK;
    rtm->rtm_pid = getpid();
    rtm->rtm_seq = 1;

    sin = (struct sockaddr_in *)(rtm + 1);
    sin->sin_family = AF_INET;
    inet_pton(AF_INET, dst, &sin->sin_addr);

    (sin + 1)->sin_family = AF_INET;
    inet_pton(AF_INET, gateway, &(sin + 1)->sin_addr);

    (sin + 2)->sin_family = AF_INET;
    inet_pton(AF_INET, netmask, &(sin + 2)->sin_addr);

    // 发送路由添加消息
    if (write(fd, buf, rtm->rtm_msglen) < 0) {
        perror("write");
        close(fd);
        exit(1);
    }

    printf("Route added: Destination=%s, Gateway=%s, Netmask=%s\n", dst, gateway, netmask);
    close(fd);
}

int main() {
    add_route("192.168.1.0", "192.168.1.1", "255.255.255.0");
    return 0;
}
```

---

### 6. **难点与注意事项**
   - **权限问题**：操作路由表需要 root 权限。
   - **消息构造**：路由消息的构造需要仔细设置各个字段。
   - **错误处理**：`write` 和 `read` 的返回值需要检查，错误时设置 `errno`。
   - **平台差异**：不同操作系统对路由套接字的支持可能有所不同。

---

### 7. **复习建议**
   - **理解路由套接字的作用和基本用法**。
   - **掌握路由消息结构**（如 `rt_msghdr` 和 `sockaddr_in`）。
   - **熟悉常见路由消息类型**（如 `RTM_ADD`、`RTM_GET`）。
   - **通过代码示例加深理解**，并尝试修改和扩展示例代码。

---

通过这个复习大纲，你可以系统地掌握第 18 章的内容。如果有进一步的问题，欢迎随时提问！
## 第十九 密钥管理 
《Unix 网络编程卷 1：套接字 API》第 3 版第 19 章主要介绍了 **密钥管理套接字（Key Management Sockets）**，这是用于管理 IPsec（IP 安全协议）密钥的机制。以下是本章的复习大纲，涵盖重要知识点和代码示例，帮助你更好地掌握内容。

---

## 第 19 章复习大纲：密钥管理套接字

### 1. **IPsec 概述**
   - **IPsec 的作用**：
     - 提供网络层的安全性，包括数据加密、数据完整性验证和身份认证。
     - 常用于 VPN（虚拟专用网络）和安全的网络通信。
   - **IPsec 的组成**：
     - **AH（认证头）**：提供数据完整性和身份认证。
     - **ESP（封装安全载荷）**：提供数据加密、完整性和身份认证。
     - **SAD（安全关联数据库）**：存储安全关联（SA）信息。
     - **SPD（安全策略数据库）**：定义数据包的处理规则。

---

### 2. **密钥管理套接字简介**
   - **密钥管理套接字的作用**：
     - 用于在用户空间和内核之间传递 IPsec 的密钥和安全关联（SA）信息。
     - 通过 `PF_KEY` 套接字族实现。
   - **PF_KEY 套接字族**：
     - 类似于 `PF_INET` 或 `PF_UNIX`，但专门用于密钥管理。
     - 使用 `SOCK_RAW` 类型创建套接字。

---

### 3. **密钥管理套接字 API**
   - **创建密钥管理套接字**：
     ```c
     int key_socket = socket(PF_KEY, SOCK_RAW, PF_KEY_V2);
     if (key_socket < 0) {
         perror("socket");
         exit(1);
     }
     ```
   - **密钥管理消息结构**：
     - 使用 `struct sadb_msg` 作为消息头。
     - 消息类型包括 `SADB_ADD`（添加 SA）、`SADB_DELETE`（删除 SA）、`SADB_GET`（获取 SA）等。
     - 示例消息结构：
       ```c
       struct sadb_msg msg;
       msg.sadb_msg_version = PF_KEY_V2;
       msg.sadb_msg_type = SADB_ADD;
       msg.sadb_msg_len = sizeof(msg) / sizeof(uint64_t);
       msg.sadb_msg_seq = 1;
       msg.sadb_msg_pid = getpid();
       ```

---

### 4. **安全关联（SA）**
   - **SA 的作用**：
     - 定义 IPsec 通信的安全参数，包括加密算法、密钥、SPI（安全参数索引）等。
   - **SA 的组成**：
     - SPI：唯一标识一个 SA。
     - 加密算法（如 AES、3DES）。
     - 认证算法（如 HMAC-SHA1）。
     - 密钥。
   - **添加 SA 的示例**：
     ```c
     struct sadb_sa sa;
     sa.sadb_sa_len = sizeof(sa) / sizeof(uint64_t);
     sa.sadb_sa_exttype = SADB_EXT_SA;
     sa.sadb_sa_spi = htonl(0x12345678);  // SPI
     sa.sadb_sa_state = SADB_SASTATE_MATURE;
     sa.sadb_sa_auth = SADB_AALG_SHA1HMAC;  // 认证算法
     sa.sadb_sa_encrypt = SADB_EALG_AES;    // 加密算法
     ```

---

### 5. **密钥管理套接字的消息类型**
   - **常见消息类型**：
     - `SADB_ADD`：添加一个新的 SA。
     - `SADB_DELETE`：删除一个 SA。
     - `SADB_GET`：获取一个 SA。
     - `SADB_ACQUIRE`：请求一个新的 SA。
     - `SADB_REGISTER`：注册支持的算法。
   - **消息的发送与接收**：
     - 使用 `sendmsg` 和 `recvmsg` 函数发送和接收密钥管理消息。
     - 示例：
       ```c
       sendmsg(key_socket, &msg, 0);
       recvmsg(key_socket, &msg, 0);
       ```

---

### 6. **密钥管理套接字的扩展**
   - **扩展类型**：
     - `SADB_EXT_SA`：安全关联扩展。
     - `SADB_EXT_ADDRESS`：地址扩展。
     - `SADB_EXT_KEY`：密钥扩展。
   - **扩展的使用**：
     - 扩展附加在消息头之后，用于传递额外的信息。

---

### 7. **密钥管理套接字的实际应用**
   - **配置 IPsec VPN**：
     - 使用密钥管理套接字配置 IPsec VPN 的安全关联和密钥。
   - **动态密钥更新**：
     - 在通信过程中动态更新密钥，提高安全性。

---

### 8. **代码示例：添加一个 SA**
```c
#include <sys/socket.h>
#include <net/pfkeyv2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    int key_socket = socket(PF_KEY, SOCK_RAW, PF_KEY_V2);
    if (key_socket < 0) {
        perror("socket");
        exit(1);
    }

    struct sadb_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.sadb_msg_version = PF_KEY_V2;
    msg.sadb_msg_type = SADB_ADD;
    msg.sadb_msg_len = sizeof(msg) / sizeof(uint64_t);
    msg.sadb_msg_seq = 1;
    msg.sadb_msg_pid = getpid();

    sendmsg(key_socket, &msg, 0);
    close(key_socket);
    return 0;
}
```

---

### 9. **难点与注意事项**
   - **内核与用户空间的交互**：
     - 密钥管理套接字是用户空间与内核之间的桥梁，理解其消息传递机制是关键。
   - **消息格式的复杂性**：
     - 密钥管理消息的格式较为复杂，需要仔细阅读文档和示例代码。
   - **IPsec 配置的复杂性**：
     - IPsec 的配置涉及多个组件（如 SA、SPD、SAD），需要全面理解其工作原理。

---

通过以上大纲和代码示例，你可以系统地复习第 19 章的内容。如果有具体问题或需要进一步的解释，请随时告诉我！ 😊
## 第二十 广播
《Unix 网络编程卷 1：套接字 API》第 3 版第 20章主要介绍了 **广播**， 如果这一章是很实用的，请帮我知知识归纳帮助 我更好地掌握内容。
《Unix 网络编程卷 1：套接字 API》第 3 版第 20 章主要介绍了 **广播（Broadcasting）**，这是一种在网络中向多个主机发送数据的通信方式。广播在实际应用中非常有用，特别是在需要向局域网内所有主机发送消息的场景中（如 DHCP、网络发现等）。

以下是第 20 章的复习大纲，帮助你更好地掌握广播的相关知识。

---

## 第 20 章复习大纲：广播

### 1. **广播的概念**
   - **广播的定义**：
     - 广播是一种向网络中所有主机发送数据的通信方式。
     - 数据包会被发送到网络中的所有主机，而不仅仅是特定的目标主机。
   - **广播的用途**：
     - 网络发现（如 DHCP 请求）。
     - 实时通知（如网络时间同步）。
     - 局域网内的群发消息。

---

### 2. **广播的类型**
   - **受限广播（Limited Broadcast）**：
     - 目标地址为 `255.255.255.255`。
     - 数据包不会被路由器转发，仅限于本地网络。
   - **定向广播（Directed Broadcast）**：
     - 目标地址为特定网络的广播地址（如 `192.168.1.255`）。
     - 数据包可以被路由器转发到指定网络。

---

### 3. **广播的套接字选项**
   - **设置套接字为广播模式**：
     - 使用 `setsockopt` 函数设置 `SO_BROADCAST` 选项。
     - 示例：
       ```c
       int broadcast = 1;
       setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
       ```

---

### 4. **广播的发送与接收**
   - **发送广播数据包**：
     - 使用 `sendto` 函数向广播地址发送数据。
     - 示例：
       ```c
       struct sockaddr_in broadcast_addr;
       memset(&broadcast_addr, 0, sizeof(broadcast_addr));
       broadcast_addr.sin_family = AF_INET;
       broadcast_addr.sin_port = htons(12345);
       inet_pton(AF_INET, "255.255.255.255", &broadcast_addr.sin_addr);

       const char *message = "Hello, Broadcast!";
       sendto(sockfd, message, strlen(message), 0,
              (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
       ```
   - **接收广播数据包**：
     - 使用 `recvfrom` 函数接收广播数据。
     - 示例：
       ```c
       char buffer[1024];
       struct sockaddr_in sender_addr;
       socklen_t addr_len = sizeof(sender_addr);

       ssize_t len = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                             (struct sockaddr *)&sender_addr, &addr_len);
       if (len < 0) {
           perror("recvfrom");
       } else {
           buffer[len] = '\0';
           printf("Received: %s\n", buffer);
       }
       ```

---

### 5. **广播的注意事项**
   - **网络负载**：
     - 广播会向网络中的所有主机发送数据，可能增加网络负载。
   - **安全性**：
     - 广播数据包可以被网络中的任何主机接收，因此不适合传输敏感信息。
   - **路由限制**：
     - 受限广播数据包不会被路由器转发，仅限于本地网络。

---

### 6. **广播的应用场景**
   - **DHCP**：
     - 客户端通过广播发送 DHCP 请求，寻找可用的 DHCP 服务器。
   - **网络时间协议（NTP）**：
     - 使用广播同步局域网内主机的时间。
   - **网络发现**：
     - 设备通过广播宣布自己的存在（如打印机、网络摄像头）。

---

### 7.  **总结**
   - 广播是一种向网络中所有主机发送数据的通信方式，适用于局域网内的群发消息。
   - 使用 `setsockopt` 设置 `SO_BROADCAST` 选项，并通过 `sendto` 和 `recvfrom` 发送和接收广播数据。
   - 广播在网络发现、DHCP 等场景中非常有用，但需要注意网络负载和安全性问题。

---

希望这个归纳总结能帮助你更好地掌握第 20 章的内容！如果还有其他问题，欢迎随时提问！ 😊 
## 第二一 多播
《Unix 网络编程卷 1：套接字 API》第 3 版第 20章主要介绍了 **多播**， 如果这一章是很实用的，请帮我知知识归纳帮助 我更好地掌握内容。
《Unix 网络编程卷 1：套接字 API》第 3 版第 21 章（而非第 20 章）主要介绍了 **多播（Multicast）**。多播是一种高效的网络通信方式，允许数据从一个发送者传输到多个接收者，而不会像广播那样影响整个网络。多播在实际应用中非常有用，特别是在视频流、在线游戏、分布式系统等场景中。

以下是第 21 章的复习大纲，帮助你更好地掌握多播的相关知识。

---

## 第 21 章复习大纲：多播

### 1. **多播的概念**
   - **多播的定义**：
     - 多播是一种向一组特定的主机发送数据的通信方式。
     - 数据包只会被发送到加入多播组的主机，而不是网络中的所有主机。
   - **多播的用途**：
     - 视频流媒体（如 IPTV）。
     - 在线游戏和实时通信。
     - 分布式系统中的数据分发。

---

### 2. **多播地址**
   - **IPv4 多播地址范围**：
     - `224.0.0.0` 到 `239.255.255.255`。
     - 其中，`224.0.0.0` 到 `224.0.0.255` 保留用于本地网络控制。
   - **IPv6 多播地址范围**：
     - `ff00::/8`。
   - **常见的多播地址**：
     - `224.0.0.1`：所有主机的多播地址。
     - `224.0.0.2`：所有路由器的多播地址。

---

### 3. **多播的套接字选项**
   - **加入多播组**：
     - 使用 `setsockopt` 函数设置 `IP_ADD_MEMBERSHIP` 选项。
     - 示例：
       ```c
       struct ip_mreq mreq;
       inet_pton(AF_INET, "239.255.255.250", &mreq.imr_multiaddr);
       mreq.imr_interface.s_addr = htonl(INADDR_ANY);
       setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
       ```
   - **离开多播组**：
     - 使用 `setsockopt` 函数设置 `IP_DROP_MEMBERSHIP` 选项。
     - 示例：
       ```c
       setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
       ```

---

### 4. **多播的发送与接收**
   - **发送多播数据包**：
     - 使用 `sendto` 函数向多播地址发送数据。
     - 示例：
       ```c
       struct sockaddr_in multicast_addr;
       memset(&multicast_addr, 0, sizeof(multicast_addr));
       multicast_addr.sin_family = AF_INET;
       multicast_addr.sin_port = htons(12345);
       inet_pton(AF_INET, "239.255.255.250", &multicast_addr.sin_addr);

       const char *message = "Hello, Multicast!";
       sendto(sockfd, message, strlen(message), 0,
              (struct sockaddr *)&multicast_addr, sizeof(multicast_addr));
       ```
   - **接收多播数据包**：
     - 使用 `recvfrom` 函数接收多播数据。
     - 示例：
       ```c
       char buffer[1024];
       struct sockaddr_in sender_addr;
       socklen_t addr_len = sizeof(sender_addr);

       ssize_t len = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                             (struct sockaddr *)&sender_addr, &addr_len);
       if (len < 0) {
           perror("recvfrom");
       } else {
           buffer[len] = '\0';
           printf("Received: %s\n", buffer);
       }
       ```

---

### 5. **多播的注意事项**
   - **TTL（Time To Live）**：
     - 设置多播数据包的 TTL，控制其传播范围。
     - 示例：
       ```c
       int ttl = 1;  // 仅限于本地网络
       setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
       ```
   - **多播环路**：
     - 控制多播数据包是否回送到发送主机。
     - 示例：
       ```c
       int loop = 1;  // 启用环路
       setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
       ```

---

### 6. **多播的应用场景**
   - **视频流媒体**：
     - 使用多播传输视频流，减少网络带宽占用。
   - **在线游戏**：
     - 使用多播同步游戏状态。
   - **分布式系统**：
     - 使用多播进行数据分发和状态同步。

---

### 7.  **总结**
   - 多播是一种高效的网络通信方式，适用于视频流、在线游戏和分布式系统等场景。
   - 使用 `setsockopt` 设置多播选项，并通过 `sendto` 和 `recvfrom` 发送和接收多播数据。
   - 多播可以显著减少网络带宽占用，但需要合理设置 TTL 和环路选项。

---

希望这个归纳总结能帮助你更好地掌握第 21 章的内容！如果还有其他问题，欢迎随时提问！ 😊
## 第二二 高级 UDP
《Unix 网络编程卷 1：套接字 API》第 3 版第 20章主要介绍了 **高级 UDP**， 如果这一章是很实用的，请帮我知知识归纳帮助 我更好地掌握内容。
《Unix 网络编程卷 1：套接字 API》第 3 版第 20 章主要介绍了 **高级 UDP 套接字编程**，涵盖了 UDP 的一些高级特性和技术，这些内容在实际开发中非常实用，特别是在需要高效、灵活的网络通信场景中。

以下是第 20 章的复习大纲，帮助你更好地掌握高级 UDP 的相关知识。

---

## 第 20 章复习大纲：高级 UDP

### 1. **UDP 的基本特性**
   - **无连接**：
     - UDP 是无连接的协议，不需要建立连接即可发送数据。
   - **不可靠性**：
     - UDP 不保证数据包的顺序、可靠性和完整性。
   - **高效性**：
     - UDP 开销小，适合对实时性要求高的应用（如视频流、在线游戏）。

---

### 2. **UDP 套接字选项**
   - **设置套接字缓冲区大小**：
     - 使用 `setsockopt` 设置发送和接收缓冲区大小。
     - 示例：
       ```c
       int send_buf_size = 1024 * 1024;  // 1 MB
       setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &send_buf_size, sizeof(send_buf_size));

       int recv_buf_size = 1024 * 1024;  // 1 MB
       setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recv_buf_size, sizeof(recv_buf_size));
       ```
   - **设置超时**：
     - 使用 `setsockopt` 设置接收超时。
     - 示例：
       ```c
       struct timeval timeout;
       timeout.tv_sec = 5;  // 5 秒
       timeout.tv_usec = 0;
       setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
       ```

---

### 3. **UDP 数据报截断**
   - **数据报截断问题**：
     - 如果接收缓冲区小于数据报大小，数据会被截断。
   - **检测截断**：
     - 使用 `recvmsg` 函数检测数据报是否被截断。
     - 示例：
       ```c
       struct msghdr msg;
       struct iovec iov;
       char buffer[1024];

       iov.iov_base = buffer;
       iov.iov_len = sizeof(buffer);

       msg.msg_name = NULL;
       msg.msg_namelen = 0;
       msg.msg_iov = &iov;
       msg.msg_iovlen = 1;
       msg.msg_control = NULL;
       msg.msg_controllen = 0;

       ssize_t len = recvmsg(sockfd, &msg, 0);
       if (len < 0) {
           perror("recvmsg");
       } else if (msg.msg_flags & MSG_TRUNC) {
           printf("Datagram truncated\n");
       } else {
           buffer[len] = '\0';
           printf("Received: %s\n", buffer);
       }
       ```

---

### 4. **UDP 连接**
   - **连接 UDP 套接字**：
     - 使用 `connect` 函数将 UDP 套接字与远程地址绑定。
     - 连接后，可以使用 `send` 和 `recv` 代替 `sendto` 和 `recvfrom`。
     - 示例：
       ```c
       struct sockaddr_in server_addr;
       memset(&server_addr, 0, sizeof(server_addr));
       server_addr.sin_family = AF_INET;
       server_addr.sin_port = htons(12345);
       inet_pton(AF_INET, "192.168.1.100", &server_addr.sin_addr);

       connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
       ```

---

### 5. **UDP 广播**
   - **广播地址**：
     - 使用 `255.255.255.255` 或特定网络的广播地址（如 `192.168.1.255`）。
   - **设置广播选项**：
     - 使用 `setsockopt` 设置 `SO_BROADCAST` 选项。
     - 示例：
       ```c
       int broadcast = 1;
       setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
       ```

---

### 6. **UDP 多播**
   - **多播地址**：
     - 使用 `224.0.0.0` 到 `239.255.255.255` 范围内的地址。
   - **加入多播组**：
     - 使用 `setsockopt` 设置 `IP_ADD_MEMBERSHIP` 选项。
     - 示例：
       ```c
       struct ip_mreq mreq;
       inet_pton(AF_INET, "239.255.255.250", &mreq.imr_multiaddr);
       mreq.imr_interface.s_addr = htonl(INADDR_ANY);
       setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
       ```

---

### 7. **UDP 的可靠性**
   - **实现可靠性的方法**：
     - 使用应用层协议实现可靠性（如重传机制、确认机制）。
     - 示例：实现简单的重传机制：
       ```c
       const char *message = "Hello, UDP!";
       for (int i = 0; i < 3; i++) {  // 重试 3 次
           if (sendto(sockfd, message, strlen(message), 0,
                     (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
               perror("sendto");
           } else {
               break;
           }
       }
       ```

---

### 8. **UDP 的性能优化**
   - **减少系统调用**：
     - 使用 `sendmmsg` 和 `recvmmsg` 批量发送和接收数据报。
   - **零拷贝技术**：
     - 使用 `splice` 或 `sendfile` 减少数据拷贝。

---

### 9. **代码示例：UDP 服务器与客户端**
以下是一个简单的 UDP 服务器与客户端示例：

#### 服务器端代码：
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[1024];

    // 创建 UDP 套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // 绑定到本地地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    // 接收数据
    socklen_t addr_len = sizeof(client_addr);
    ssize_t len = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                           (struct sockaddr *)&client_addr, &addr_len);
    if (len < 0) {
        perror("recvfrom");
    } else {
        buffer[len] = '\0';
        printf("Received: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
```

#### 客户端代码：
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    const char *message = "Hello, UDP!";

    // 创建 UDP 套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // 发送数据
    if (sendto(sockfd, message, strlen(message), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("sendto");
        close(sockfd);
        exit(1);
    }

    printf("Message sent: %s\n", message);
    close(sockfd);
    return 0;
}
```

---

### 10. **总结**
   - 高级 UDP 编程涉及套接字选项、数据报截断、连接、广播、多播等技术。
   - 通过合理使用这些技术，可以优化 UDP 的性能和可靠性。
   - UDP 适合对实时性要求高的应用，但需要开发者自行处理可靠性和错误恢复。

---

希望这个归纳总结能帮助你更好地掌握第 20 章的内容！如果还有其他问题，欢迎随时提问！ 😊
## 第二三 高级 sctp
《Unix 网络编程卷 1：套接字 API》第 3 版第 20章主要介绍了 **高级 sctp**， 如果这一章是很实用的，请帮我知知识归纳帮助 我更好地掌握内容。

## 二十四 带外

## 二十五 信号驱动

## 二十六 线程

## 二十七 IP

## 二十八 原始套接字

## 二十九 数据链路访问 

## 三十章 客户服务器设计模式 

## 三十一 流
《Unix 网络编程卷 1：套接字 API》第 3 版第 31章主要介绍了 **流**，我不太明白为什么 称为流，我知道 这肯定 不是指tcp stream, 而是一项比较 新的协议 ？ 我从业 没有听说过，它不是很有用对吧。如果这一章是很实用的，请帮我知知识归纳帮助 我更好地掌握内容。
感谢你的指正！确实，书中提到的 **流（Streams）** 不仅仅是简单的消息传递机制，还涉及更复杂的 API 和协议接口，如 `getmsg`、`putmsg`、**TPI（Transport Provider Interface）** 和 **XTI（X/Open Transport Interface）**。以下是对这些内容的详细归纳和总结，帮助你更好地理解第 31 章的内容。

---

## 第 31 章复习大纲：流（Streams）与 TPI/XTI

### 1. **流的概念**
   - **流的定义**：
     - 流是 Unix 系统中一种基于消息的通信机制，允许内核和用户空间之间传递数据。
     - 它提供了一种模块化的方式来处理数据，类似于管道（pipe），但更灵活。
   - **流的特点**：
     - 数据以消息（message）的形式传递。
     - 支持多路复用（multiplexing）。
     - 可以在流中插入处理模块（module），对数据进行过滤或转换。

---

### 2. **流的组成**
   - **流头（Stream Head）**：
     - 用户空间和内核之间的接口。
   - **驱动（Driver）**：
     - 流的底层实现，负责与硬件或其他内核模块通信。
   - **模块（Module）**：
     - 可以在流中插入的中间处理层，用于对数据进行过滤、转换或增强。

---

### 3. **流的 API**
   - **`getmsg` 和 `putmsg`**：
     - 用于从流中读取消息（`getmsg`）和向流中写入消息（`putmsg`）。
     - 这些函数允许用户空间程序直接操作流中的消息。
     - 示例：
       ```c
       struct strbuf ctl, data;
       int flags;

       // 读取消息
       getmsg(fd, &ctl, &data, &flags);

       // 写入消息
       putmsg(fd, &ctl, &data, flags);
       ```

---

### 4. **TPI（Transport Provider Interface）**
   - **TPI 的作用**：
     - TPI 是流的一种实现，用于定义传输层协议（如 TCP、UDP）的接口。
     - 它允许用户空间程序直接与传输层协议交互。
   - **TPI 的特点**：
     - 提供了一种标准化的方式来实现传输层协议。
     - 支持面向连接（如 TCP）和无连接（如 UDP）的通信。

---

### 5. **XTI（X/Open Transport Interface）**
   - **XTI 的作用**：
     - XTI 是基于 TPI 的高级接口，提供了更简单的 API 来实现传输层协议。
     - 它是 TPI 的标准化版本，通常用于实现网络协议栈。
   - **XTI 的特点**：
     - 提供了与套接字 API 类似的功能，但更底层。
     - 支持面向连接和无连接的通信。

---

### 6. **流与 TPI/XTI 的关系**
   - **流是底层机制**：
     - 流提供了消息传递的基础设施。
   - **TPI/XTI 是上层接口**：
     - TPI/XTI 基于流实现，提供了传输层协议的接口。

---

### 7. **流的应用场景**
   - **网络协议栈**：
     - 流可以用于实现网络协议栈，每个协议层作为一个模块插入流中。
   - **设备驱动**：
     - 流可以用于实现设备驱动，允许用户空间和硬件之间传递数据。
   - **数据过滤**：
     - 流可以用于实现数据过滤或转换，例如加密、压缩等。

---

### 8. **代码示例：使用 `getmsg` 和 `putmsg`**
以下是一个简单的示例，展示如何使用 `getmsg` 和 `putmsg`：
```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stropts.h>

int main() {
    int fd = open("/dev/stream_device", O_RDWR);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    struct strbuf ctl, data;
    char ctl_buf[1024], data_buf[1024];
    int flags = 0;

    ctl.buf = ctl_buf;
    ctl.maxlen = sizeof(ctl_buf);
    data.buf = data_buf;
    data.maxlen = sizeof(data_buf);

    // 读取消息
    if (getmsg(fd, &ctl, &data, &flags) < 0) {
        perror("getmsg");
        close(fd);
        exit(1);
    }

    printf("Received control message: %s\n", ctl.buf);
    printf("Received data message: %s\n", data.buf);

    // 写入消息
    ctl.len = snprintf(ctl.buf, sizeof(ctl_buf), "Control Message");
    data.len = snprintf(data.buf, sizeof(data_buf), "Data Message");

    if (putmsg(fd, &ctl, &data, flags) < 0) {
        perror("putmsg");
        close(fd);
        exit(1);
    }

    close(fd);
    return 0;
}
```

---

### 9. **难点与注意事项**
   - **流的复杂性**：
     - 流的设计和实现较为复杂，需要深入理解内核和用户空间的交互。
   - **TPI/XTI 的使用**：
     - TPI/XTI 提供了底层的传输层接口，但使用起来比套接字 API 更复杂。
   - **兼容性问题**：
     - 流和 TPI/XTI 在现代操作系统中逐渐被淘汰，可能缺乏相关支持和文档。

---

### 10. **总结**
   - 流是 Unix 系统中一种基于消息的通信机制，具有模块化和多路复用的特点。
   - TPI 和 XTI 是基于流的传输层接口，提供了底层的网络协议实现方式。
   - 尽管流和 TPI/XTI 在现代网络中不常见，但理解其设计思想对深入理解 Unix 系统编程有帮助。

---

希望这个归纳总结能帮助你更好地掌握第 31 章的内容！如果还有其他问题，欢迎随时提问！ 😊