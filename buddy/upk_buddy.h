
#include <sqlite3.h>

/* 
   Create a buddies to start and watch a processe.

   todo: less args
   todo: list return type / multiple arg
*/
int upk_buddy_start(
                    upk_srvc_t    srvc,
                    const char    *command,
                    const char    *env[]

);

/* 
 * stop the application that buddy was watching.
 * 
 */
int upk_buddy_stop( int pid );                   


int upk_service_buddy_stop( upk_srvc_t s );                   

typedef enum { BUDDY_STATUS, BUDDY_EXIT } upk_buddy_message;

/*
 * Sends a message to a buddy, either by service or pid
 */
int upk_service_buddy_send_message (upk_srvc_t s,upk_buddy_message m) ;
int upk_buddy_send_message (int bpid,upk_buddy_message m) ;

/* 
 * Returns a socket connected to a buddy
 */
int upk_buddy_connect (int bpid) ;
int upk_service_buddy_connect (upk_srvc_t srvc);
