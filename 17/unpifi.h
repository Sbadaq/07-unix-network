#ifndef __unp_ifi_h
#define __unp_ifi_h

#include <net/if.h>

#define IFI_NAME    16
#define IFI_HADDR   8

struct ifi_info {
    char    ifi_name[IFI_NAME];     /* interface name, null-terminated */
    short   ifi_index;              /* interface index */
    short   ifi_mtu;                /* interface MTU */
    u_char  ifi_haddr[IFI_HADDR];   /* hardware address */
    u_char  ifi_hlen;               /* #bytes in hardware address: 0, 6 */
    short   ifi_flags; 
    short   ifi_myflags;
    struct  sockaddr     *ifi_addr;
    struct  sockaddr     *ifi_broadaddr;
    struct  sockaddr     *ifi_dstaddr;
    struct  ifi_info *ifi_next;
};
#define IFI_ALIAS       1

struct ifi_info *get_ifi_info(int, int);
struct ifi_info *get_ifi_info_by_name(char *);
void free_ifi_info(struct ifi_info *);
#endif