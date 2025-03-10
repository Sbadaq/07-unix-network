
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/mman.h>


#define max(a,b) ((a) > (b) ? (a) : (b))

char* gf_time()
{
    time_t t;
    struct tm *tm;
    char *buf;
    time(&t);
    tm = localtime(&t);
    buf = asctime(tm);
    return buf;
}

void str_cli(FILE *fp, int sockfd)
{
    int maxfdp1,val,stdineof;
    ssize_t n,nwritten;

    fd_set rset,wset;
    char to[MAXLINE],from[MAXLINE];
    char *toiptr,*tooptr,*fromiptr,*fromoptr;
    /* set nonblocking */
    val = fcntl(sockfd,F_GETFL,0);
    fcntl(sockfd,F_SETFL,val|O_NONBLOCK);

    val = fcntl(STDIN_FILENO,F_GETFL,0);
    fcntl(STDIN_FILENO,F_SETFL,val|O_NONBLOCK);

    val = fcntl(STDOUT_FILENO,F_GETFL,0);
    fcntl(STDOUT_FILENO,F_SETFL,val|O_NONBLOCK);
    /* initialize two buffer pointers */
    toiptr = tooptr = to;
    fromiptr = fromoptr = from;

    stdineof = 0;

    maxfdp1 = max(STDOUT_FILENO,STDIN_FILENO) + 1;
    /* select loop */
    for ( ; ; ) {
        /* two set reset and open at most two fd */
        FD_ZERO(&rset);
        FD_ZERO(&wset);

        if(stdineof == 0 && toiptr < &to[MAXLINE])
            FD_SET(STDIN_FILENO,&rset);
        if(fromoptr < &from[MAXLINE])
            FD_SET(sockfd,&rset);
        if(tooptr != toiptr)
            FD_SET(sockfd,&wset);
        if(fromiptr != fromoptr)
            FD_SET(STDOUT_FILENO,&wset);

        /* wait one of four fd to be true */
        if(select(maxfdp1,&rset,&wset,NULL,NULL) < 0)
            err_sys("select error");
        /* read from stdin */
        if(FD_ISSET(STDIN_FILENO,&rset)) {
            /* the 3rd param is the capacity of buffer */
            if((n = read(STDIN_FILENO,toiptr,&to[MAXLINE] - toiptr)) < 0)
                /* if the error is EWOULDBLOCK, ignore it */
                if (errno != EWOULDBLOCK) {
                    /* we say that would not happen! */
                    err_sys("read error");
                }
            else if(n == 0) {
                stdineof = 1;
                /* if the buffer is empty, send FIN */
                if(tooptr == toiptr)
                    shutdown(sockfd,SHUT_WR);//send FIN
            } else{
                /* when read success, print the read bytes */
                fprintf(stderr,"%s : read %d bytes from stdin \n",gf_time(),n);
                toiptr += n;
                FD_SET(sockfd,&wset);
            }
        }
        /* read from socket */
        if(FD_ISSET(sockfd,&rset)) {
            if((n = read(sockfd,fromiptr,&from[MAXLINE] - fromiptr)) < 0)
                if (errno != EWOULDBLOCK) {
                    err_sys("read error");
                }
            else if(n == 0) {
                fprintf(stderr,"%s : read 0 bytes from socket \n",gf_time());
                if(stdineof== 1)
                    return;
                err_quit("server terminated prematurely");
            }else{
                fprintf(stderr,"%s : read %d bytes from socket \n",gf_time(),n);
                fromiptr += n;
                FD_SET(STDOUT_FILENO,&wset);
            }
        }
        /* if output is available and data > 0 */
        if(FD_ISSET(STDOUT_FILENO,&wset) && ( (n = toiptr - tooptr) > 0)){
            if((nwritten = write(STDOUT_FILENO,tooptr,n)) < 0){
                /* here is possibble */
                if(errno == EWOULDBLOCK)
                    continue;
                else
                    err_sys("write error");
            }else{
                /* write success , fromoptr add,*/
                fprintf(stderr,"%s : write %d bytes to stdout \n",gf_time(),nwritten);
                fromoptr += nwritten;
                /* if  back to begin of the buff*/
                if(fromoptr == fromiptr){
                    fromoptr = fromiptr = from;
                    if(stdineof)
                        shutdown(sockfd,SHUT_WR);
                }
                    
            }
        }
        if(FD_ISSET(sockfd,&wset) && ( (n = toiptr - tooptr) > 0)){
            if((nwritten = write(sockfd,tooptr,n)) < 0){
                if(errno == EWOULDBLOCK)
                    continue;
                else
                    err_sys("write error");
            }
            else{
                fprintf(stderr,"%s : write %d bytes to socket \n",gf_time(),nwritten);
                tooptr += nwritten;
                if(tooptr == toiptr)
            }
        }
    }
}