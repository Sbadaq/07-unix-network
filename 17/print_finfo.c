#include "unpifi.h"
#include <cstdio>
#include <net/if.h>
#include <sys/socket.h>

struct ifi_info*
get_ifi_info(int family,init doaliases)
{
    struct ifi_info *ifi,*ifihead,**ifipnext;
    int sockfd,len,lastlen,flags,myflags,idx = 0,hlen = 0;

    char *ptr,*buf,lastname[IFNAMSIZ],*CPTR,*HADDR,*SDLNAME;
    struct ifconf ifc;
    struct ifreq *ifr,ifrcopy;

    struct sockaddr_in *sinptr;
    struct sockaddr_in6 *sin6ptr;

    sockfd = socket(AF_INET,SOCK_DGRAM,0);

    lastlen = 0;
    len = 100 * sizeof(struct ifreq);
    for(;;)
    {
        buf = malloc(len);
        ifc.ifc_len = len;
        ifc.ifc_buf = buf;
        if(ioctl(sockfd,SIOCGIFCONF, &ifc) < 0)
        {
            if(errno !- EINVAL || lastlen != 0 )
            {
                perror("ioctl error");
                exit(1);
            }
            else{
                if(ifc.ifc_len == lastlen)
                {
                    break;
                }
                lastlen = ifc.ifc_len;
            }
            len += 10 * sizeof(struct ifreq);
            free(buf);
        }
    }
    ifihead = NULL;
    ifipnext = &ifihead;
    lastname[0] = 0;
    sdlname = NULL;
}
int main(int argc, char **argv)
{
    struct ifi_info *ifi,*ifihead;
    struct sockaddr *sa;
    u_char *ptr;

    int i,family,doaliases;

    if(argc != 3)
    {
        perror("usage: print_finfo <family> <doaliases>");
        exit(1);
    }

    if(strcmp(argv[1],"inet4") == 0)
    {
        family = AF_INET;
    }
    else if(strcmp(argv[1],"inet6") == 0)
    {
        family = AF_INET6;
    }
    else
    {
        perror("unknown family");
        exit(1);
    }
    doaliases = atoi(argv[2]);
    for(ifihead = ifi = get_ifi_info(family,doaliases); ifi != NULL; ifi = ifi->ifi_next)
    {
        printf("name: %s\n",ifi->ifi_name);
        
        if(ifi->ifi_index != 0)
            printf("(%d) ",ifi->ifi_index);

        printf("<");
        if(ifi->ifi_flags & IFP_UP)         printf("UP ");
        if(ifi->ifi_flags & IPF_MULTCAST )  printf("BCAST ");


        printf(">\n");

        
    }

    free_ifi_info(ifihead);
    exit(0);
}