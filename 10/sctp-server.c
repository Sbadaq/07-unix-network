#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <netinet/sctp.h>


int main()
{
    int sock_fd,msg_flags;
    char read_buf[1024];
    struct sockaddr_in server,client;
    struct sctp_sndrcvinfo sri;
    struct sctp_event_subscribe event;
    struct sctp_initmsg initmsg;
    socklen_t len = sizeof(client);
    int streamid,ppid;
    int stream_increment = 1;
    char *msg="Hello";
    
    if((sock_fd=socket(AF_INET,SOCK_SEQPACKET,IPPROTO_SCTP))<0)
    {
        perror("Socket creation failed");
        exit(1);
    }
    
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    bind(sock_fd,(struct sockaddr *)&server,sizeof(server));

    memset(&event, 0, sizeof(event));
    event.sctp_data_io_event = 1;
    setsockopt(sock_fd, IPPROTO_SCTP, SCTP_EVENTS, &event, sizeof(event));
    listen(sock_fd, 5);
    for (; ; )
    {
        int len = sizeof(struct sockaddr_in);
        int rd_sz = sctp_recvmsg(sock_fd, read_buf, sizeof(read_buf), &msg_flags,
                (struct sockaddr *)&client, &len, &sri);
            
        if(stream_increment)
        {
            sri.sinfo_stream ++;
            if(sri.sinfo_stream >= sri.sinfo_stream)
            {
                stream_increment = 0;
            }

        }   
        sctp_sendmsg(sock_fd, msg, strlen(msg), NULL, 0, client.sin_addr.s_addr, client.sin_port, sri.sinfo_stream, 0, 0);
    }
}