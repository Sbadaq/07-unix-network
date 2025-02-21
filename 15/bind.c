
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/un.h>

int main(int argc, char **argv)
{
    int sockfd;
    socklen_t len;
    struct sockaddr_un addr1;
    struct sockaddr_un addr2;
    if(argc != 2)
    {
        printf("Usage: unixbind <pathname>");
        return 1;
    }
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    unlink(argv[1]);//ok if this fails
    memset(&addr1, 0, sizeof(struct sockaddr_in));
    addr1.sun_family = AF_UNIX;
    strncpy(addr1.sun_path, argv[1], sizeof(addr1.sun_path) - 1);
    bind(sockfd, (struct sockaddr *)&addr1, sizeof(struct sockaddr_un));
    len = sizeof(struct sockaddr_un);
    getsockname(sockfd, (struct sockaddr *)&addr2, &len);
    printf("bound name = %s\n", addr2.sun_path);
    return 0;
}