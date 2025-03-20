#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

 

static void doit(void *arg);
static int sockfd;
static FILE* fp;

void str_cli(FILE* fp_arg, int sockfd_arg)
{

}

void copyto(void *arg)
{

}

int main(int argc, char **argv)
{
    int listenfd,connfd;
    pthread_t tid;
    socklen_t addrlen,len;
    struct sockaddr *cliaddr;

    if(argc == 2)
    {
        listenfd = tcp_listen(NULL, argv[1], &addrlen);
    }
    else if (argc == 3)
    {
        listenfd = tcp_listen(argv[1], argv[2], &addrlen);
    }
    else
    {
        err_quit("usage: tcpcli01 [ <host> ] <port#>");
    }
    cliaddr = Malloc(addrlen);
    for(;;)
    {
        len = addrlen;
        connfd = accept(listenfd, cliaddr, &len);
        if(connfd < 0)
        {
            err_sys("accept error");
        }
        if(pthread_create(&tid, NULL, copyto, (void*)connfd) != 0)
        {
            err_sys("pthread_create error");
        }
    }
}

static void doit(void *arg)
{
    pthread_detach(pthread_self());
    str_echo((int) arg);
    close((int)arg);
    return (NULL); 
}