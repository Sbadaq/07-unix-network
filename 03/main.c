#include <netinet/in.h>

int main()
{
    const char* ip_str = "192.168.1.1";
    struct in_addr addr;
    inet_pton(AF_INET, ip_str, &addr);
    printf("IP: %u\n", addr.s_addr);
    char ip_str2[128];
    inet_ntop(AF_INET, &addr, ip_str2, INET_ADDRSTRLEN);
    printf("IP: %s\n", ip_str2);
    return 0;
}