
#include <sqlite3.h>

/* 
   Create a buddies to start and watch a processe.

   todo: less args
   todo: list return type / multiple arg
*/
int upk_buddy_start(
                    upk_srvc_t    srvc,
                    const char    *command,
                    const char    *env[],
                    const char    *db

);

/* 
   stop the application that buddy was watching.
 */
int upk_buddy_stop( upk_srvc_t s );                   


