#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

// 添加 ARP 缓存条目
int add_arp_entry(const char *ifname, const char *ip, const char *mac) {
    int fd;
    struct arpreq arp_req;
    struct sockaddr_in *sin;

    // 创建套接字
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    // 初始化 arpreq 结构
    memset(&arp_req, 0, sizeof(arp_req));

    // 设置接口名称
    strncpy(arp_req.arp_dev, ifname, IFNAMSIZ - 1);

    // 设置 IP 地址
    sin = (struct sockaddr_in *)&arp_req.arp_pa;
    sin->sin_family = AF_INET;
    if (inet_pton(AF_INET, ip, &sin->sin_addr) <= 0) {
        perror("inet_pton");
        close(fd);
        return -1;
    }

    // 设置 MAC 地址
    unsigned char mac_addr[6];
    sscanf(mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &mac_addr[0], &mac_addr[1], &mac_addr[2],
           &mac_addr[3], &mac_addr[4], &mac_addr[5]);
    memcpy(arp_req.arp_ha.sa_data, mac_addr, 6);
    arp_req.arp_ha.sa_family = ARPHRD_ETHER;

    // 设置标志
    arp_req.arp_flags = ATF_PERM | ATF_COM; // 永久条目

    // 调用 ioctl 添加 ARP 条目
    if (ioctl(fd, SIOCSARP, &arp_req) < 0) {
        perror("ioctl(SIOCSARP)");
        close(fd);
        return -1;
    }

    printf("ARP entry added: IP=%s, MAC=%s\n", ip, mac);
    close(fd);
    return 0;
}

// 删除 ARP 缓存条目
int delete_arp_entry(const char *ifname, const char *ip) {
    int fd;
    struct arpreq arp_req;
    struct sockaddr_in *sin;

    // 创建套接字
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    // 初始化 arpreq 结构
    memset(&arp_req, 0, sizeof(arp_req));

    // 设置接口名称
    strncpy(arp_req.arp_dev, ifname, IFNAMSIZ - 1);

    // 设置 IP 地址
    sin = (struct sockaddr_in *)&arp_req.arp_pa;
    sin->sin_family = AF_INET;
    if (inet_pton(AF_INET, ip, &sin->sin_addr) <= 0) {
        perror("inet_pton");
        close(fd);
        return -1;
    }

    // 调用 ioctl 删除 ARP 条目
    if (ioctl(fd, SIOCDARP, &arp_req) < 0) {
        perror("ioctl(SIOCDARP)");
        close(fd);
        return -1;
    }

    printf("ARP entry deleted: IP=%s\n", ip);
    close(fd);
    return 0;
}

int main() {
    const char *ifname = "enp0s8"; // 网络接口名称
    const char *ip = "192.168.1.100"; // 目标 IP 地址
    const char *mac = "00:11:22:33:44:55"; // 目标 MAC 地址

    // 添加 ARP 条目
    if (add_arp_entry(ifname, ip, mac) < 0) {
        fprintf(stderr, "Failed to add ARP entry\n");
        return 1;
    }

    // 删除 ARP 条目
    if (delete_arp_entry(ifname, ip) < 0) {
        fprintf(stderr, "Failed to delete ARP entry\n");
        return 1;
    }

    return 0;
}