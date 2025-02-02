/**
 * @brief Function for simple tcp server using select 
 * @author Haolaoshi
 * @addindex: http://www.
 * @date 2025-02-01 00
 *  It works in a non-blocking, event-driven way, allowing the server to manage several clients concurrently.
 */


/**
 * @brief Function for simple tcp server using select
 * @author Haolaoshi
 * @date 2025-02-01 08:24:15
 * 
 * @param 
 * 第一步，创建并绑定套接字
 * 第二步，将监听套接字添加到读集合中 readfds 
 * 第三步，调用 select 函数，等待客户端连接
 * 第四步，如果监听套接字有数据可读，接受新连接并将新连接套接字添加到读集合readfds中
 * 第五步，如果客户端套接字有数据可读，接收数据并处理
 */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>

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