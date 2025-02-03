#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/sctp.h>

void sctpstr_cli(FILE *input, int sock_fd, struct sockaddr *server, socklen_t server_len) {
    struct sctp_sndrcvinfo sndrcv;
    int flags = 0;
    char *buffer = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&buffer, &len, input)) > 0) {
        memset(&sndrcv, 0, sizeof(sndrcv));
        sndrcv.sinfo_ppid = htonl(1);

        // 使用 sctp_sendmsg 函数发送数据
        if (sctp_sendmsg(sock_fd, buffer, read, NULL, 0, sndrcv.sinfo_ppid, 0, sndrcv.sinfo_stream, 0, 0) < 0) {
            perror("sctp_sendmsg");
            return;
        }

        free(buffer);
        buffer = NULL; // 重置 buffer 指针
    }
}

int main(int argc, char **argv) {
    int sock_fd;
    struct sockaddr_in server;
    struct sctp_event_subscribe subscriber;
    int echo_to_all = 0;

    // 创建 SCTP 套接字
    sock_fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
    if (sock_fd < 0) {
        perror("socket");
        return 1;
    }

    // 初始化服务器地址结构
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 设置 SCTP 事件订阅
    memset(&subscriber, 0, sizeof(subscriber));
    subscriber.sctp_data_io_event = 1;
    if (setsockopt(sock_fd, IPPROTO_SCTP, SCTP_EVENTS, &subscriber, sizeof(subscriber)) < 0) {
        perror("setsockopt");
        close(sock_fd);
        return 1;
    }

    if (argc > 1) {
        echo_to_all = 1;
    }

    if (echo_to_all == 0) {
        sctpstr_cli(stdin, sock_fd, (struct sockaddr *)&server, sizeof(server));
    } else {
        // 如果不需要该功能，可以删除此部分
        fprintf(stderr, "sctpstr_cli_echo_all function is not implemented.\n");
    }

    // 关闭套接字
    close(sock_fd);

    return 0;
}
