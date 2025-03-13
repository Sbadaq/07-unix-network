#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// 获取所有网络接口的 IP 地址
void get_all_ips() {
    int fd;
    struct ifconf ifc;
    struct ifreq *ifr;
    char buf[1024];
    int num_ifaces;

    // 创建套接字
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        exit(1);
    }

    // 获取接口列表
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
        perror("ioctl(SIOCGIFCONF)");
        close(fd);
        exit(1);
    }

    // 遍历接口
    ifr = ifc.ifc_req;
    num_ifaces = ifc.ifc_len / sizeof(struct ifreq);
    for (int i = 0; i < num_ifaces; i++) {
        struct ifreq *item = &ifr[i];
        struct sockaddr_in *sin = (struct sockaddr_in *)&item->ifr_addr;

        // 获取 IP 地址
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip));

        // 获取硬件地址
        struct arpreq arp_req;
        memset(&arp_req, 0, sizeof(arp_req));
        memcpy(&arp_req.arp_pa, &item->ifr_addr, sizeof(struct sockaddr_in));
        arp_req.arp_ha.sa_family = ARPHRD_ETHER;

        if (ioctl(fd, SIOCGARP, &arp_req) < 0) {
            printf("Interface: %s, IP: %s, MAC: (Not available)\n", item->ifr_name, ip);
        } else {
            unsigned char *mac = (unsigned char *)arp_req.arp_ha.sa_data;
            printf("Interface: %s, IP: %s, MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                   item->ifr_name, ip, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        }
    }

    close(fd);
}

int main() {
    // 获取并显示所有 IP 地址及其硬件地址
    get_all_ips();
    return 0;
}