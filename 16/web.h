#include <sys/select.h>
#define MAXFILES    20
#define SERV        "80"

struct file{
    char *f_name;
    char *f_host;
    int f_fd;
    int f_flags;
}file[1024];

#define F_CONNECTING    1  /*CONNECT IN PROGRESS*/   
#define F_READING       2  /*CONNECT COMPLETE! , NOW READING */
#define F_DONE          4  /* all done. */

#define GET_CMD         "GET %s HTTP/1.0\r\n\r\n"
/* globals */
int nconn,nfiles,nlefttoconn,nlefttoread,maxfd;
fd_set rset,wset;
/* function prototype */
void home_page(const char* , const char* );
void start_connect(struct file*);
void write_get_cmd(struct file*);