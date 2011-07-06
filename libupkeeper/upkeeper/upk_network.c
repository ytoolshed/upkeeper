#include "upk_include.h"

int upk_client_conn_fd = -2;
int upk_neg_protocol = 0;

int
upk_client_connect(const char *sockpath)
{
    struct sockaddr_un sa = { 0 };
    int sa_len = sizeof(sa), fd = -2;

    strncpy(sa.sun_path, sockpath, sizeof(sa.sun_path) - 1);
    sa.sun_family = AF_UNIX;

    fd = socket(PF_UNIX, SOCK_STREAM, 0);

    if( connect(fd, (struct sockaddr *) sa, sa_len) != 0 )
        return -2;

    return fd;
}

 
/** 
   @brief called from event_hook, retries controller, once per spin; rate-limited in eventloop
   */ 
int 
upk_client_retry(int retries, const char *sockpath) 
{ 
        static int retry = 0; 
         
        if(retry <= retries) {
            return upk_client_connect(sockpath);
        }
        else {
            errno = EAGAIN;
            retry = 0;
        }
        return -3;
}










