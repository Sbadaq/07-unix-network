#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

/**
 * @brief 实现一个字符串客户端功能，向服务器发送字符串并接收服务器的响应
 * 
 * @param fp 文件指针，通常用于标准输入（stdin）
 * @param sockfd 套接字描述符，用于与服务 器通信
 */
void str_cli(FILE *fp, int sockfd)
{
    pid_t pid;
    char sendLine[1024],recvline[1024];

    // 创建一个子进程，子进程负责接收服务器的响应并输出到标准输出
    if( (pid = fork()) == 0) {/* iam child */
        // 子进程循环读取服务器发送的数据，并输出到标准输出
        while(read(sockfd,recvline,1024) > 0)
            fputs(recvline,stdout);
        // 当服务器断开连接时，子进程发送信号终止父进程
        kill(getppid(),SIGTERM);/* kill parent */
        // 子进程退出
        exit(0);
    }

    // 父进程：从标准输入读取数据并发送到服务器
    while(fgets(sendLine,1024,fp) != NULL)
        write(sockfd,sendLine,strlen(sendLine));
    // 父进程完成数据发送后，关闭套接字的写入功能
    shutdown(sockfd,SHUT_WR);
    // 父进程等待信号或中断
    pause();
    return;
}  
 