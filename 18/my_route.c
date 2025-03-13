#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/rtnetlink.h>

// 处理路由消息
void handle_route_message(struct nlmsghdr *nlh) {
    if (nlh->nlmsg_type == RTM_NEWROUTE) {
        struct rtmsg *rtm = NLMSG_DATA(nlh);
        char dst[INET_ADDRSTRLEN] = {0};
        char gateway[INET_ADDRSTRLEN] = {0};

        struct rtattr *rta = RTM_RTA(rtm);
        int rtl = RTM_PAYLOAD(nlh);

        for (; RTA_OK(rta, rtl); rta = RTA_NEXT(rta, rtl)) {
            switch (rta->rta_type) {
                case RTA_DST: {
                    struct in_addr *addr = (struct in_addr *)RTA_DATA(rta);
                    inet_ntop(AF_INET, addr, dst, sizeof(dst));
                    break;
                }
                case RTA_GATEWAY: {
                    struct in_addr *addr = (struct in_addr *)RTA_DATA(rta);
                    inet_ntop(AF_INET, addr, gateway, sizeof(gateway));
                    break;
                }
            }
        }

        printf("Destination: %s, Gateway: %s\n", dst, gateway);
    }
}

// 查询路由表
void query_route_table() {
    int sockfd;
    struct sockaddr_nl local;
    struct msghdr msg;
    struct iovec iov;
    struct nlmsghdr *nlh;
    char buffer[16384]; // 16KB 缓冲区

    // 创建 netlink 套接字
    sockfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sockfd < 0) {
        perror("socket");
        return;
    }

    // 绑定本地地址
    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_pid = getpid();
    local.nl_groups = 0;

    if (bind(sockfd, (struct sockaddr *)&local, sizeof(local)) < 0) {
        perror("bind");
        close(sockfd);
        return;
    }

    // 构建 netlink 请求消息
    nlh = (struct nlmsghdr *)buffer;
    nlh->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    nlh->nlmsg_type = RTM_GETROUTE;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq = 1;
    nlh->nlmsg_pid = getpid();

    struct rtmsg *rtm = NLMSG_DATA(nlh);
    rtm->rtm_family = AF_INET;
    rtm->rtm_table = RT_TABLE_MAIN;

    // 初始化消息结构
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&local;
    msg.msg_namelen = sizeof(local);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    // 发送请求消息
    printf("Sending Netlink request...\n");
    if (sendmsg(sockfd, &msg, 0) < 0) {
        perror("sendmsg");
        close(sockfd);
        return;
    }
    printf("Netlink request sent.\n");

    // 接收响应消息
    while (1) {
        printf("Waiting for Netlink response...\n");
        ssize_t n = recvmsg(sockfd, &msg, 0);
        if (n < 0) {
            perror("recvmsg");
            break;
        }
        printf("Received %zd bytes of Netlink response.\n", n);

        for (nlh = (struct nlmsghdr *)buffer; NLMSG_OK(nlh, n); nlh = NLMSG_NEXT(nlh, n)) {
            if (nlh->nlmsg_type == NLMSG_DONE) {
                printf("End of Netlink response.\n");
                break;
            }
            handle_route_message(nlh);
        }
        if (nlh->nlmsg_type == NLMSG_DONE) {
            break;
        }
    }

    close(sockfd);
}

int main() {
    query_route_table();
    return 0;
}