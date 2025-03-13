#include "web.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/select.h>
#include <unistd.h>

void home_page(const char *host, const char *fname)
{
    int fd,n;
    char line[1024];
    fd = tcp_connect(host,SERV);
    n = snprintf(line, sizeof(line), GET_CMD, fname);
    write(fd, line, n);
    for(;;)
    {
        if( ( n = read(fd,line,1024) ) == 0)
        {
            break;/* server closed connection */
        }
        printf("read %d bytes\n");
        /* do whatever with data*/
    }
    printf("end - of file on home page\n");
    close(fd);
}
int main(int argc,char** argv)
{
    int i, fd, n , maxnconn, flags, error;
    char buf[1024];
    fd_set rs,ws;
    if(argc < 5)
    {
        perror("usage : web <#conns> <hostname> <homepage> <file11> ...");
        exit(1);
    }

    maxnconn = atoi(argv[1]);
    nfiles = min(argc - 4 , 1024);
    for( i = 0 ; i < nfiles; i++)
    {
        file[i].f_name = argv[i + 4];
        file[i].f_host = argv[2];
        file[i].f_flags = 0;
    }

    printf("nfiles = %d \n",nfiles);
    home_page(argv[2], argv[3]);
    FD_ZERO(&rset);
    FD_ZERO(&wset);

    maxfd = -1;
    nlefttoread = nlefttoconn = nfiles;
    nconn = 0;

    return 0;
}