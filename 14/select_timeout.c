

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serveraddr.sin_addr);//设置服务器IP地址

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    int ret = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
    if (ret == -1) {
        perror("select");
    }else if(ret == 0) {
        printf("timeout\n");
    }else{
        printf("connect success\n");
    }
    close(sockfd);

    return 0;
}