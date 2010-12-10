#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <errno.h>
#include <string.h>

#include "store/upk_db.h"
#include "upk_buddy.h"
#define BUDDYPATH "./buddy"

extern int DEBUG;

/* 
 * Launches the application process in argv[] with a 'buddy'
 * which updates the upkeeper database when the application has been started
 * and when it got stopped or crashed.
 */

int upk_buddy_start_1(
  upk_srvc_t    srvc,                    
  const char    *command,
  const char    *env[],
  int async,
  const char    *socket
) {
  int   bpid;
  char  c;
  const char *default_socket = "./controller";

  if( DEBUG ) {
      printf("upk_buddy_start '%s' '%s' [%s] async=%d\n",
              srvc->service, srvc->package, command, async);
  }

  bpid = fork();
  
  if (bpid == -1) { 
    return -1;
  }

  if (bpid == 0) {
    execle(BUDDYPATH, 
           BUDDYPATH, 
           "-s",
           socket ? socket : default_socket,
           "-f",
           "........................................",
           command, 
           NULL,
           env);

    if( DEBUG ) {
        printf("execle %s [%s] failed (%s)\n",
                BUDDYPATH, command, strerror( errno ) );
    }

    _exit(112);
  }
  
  upk_db_service_buddy_pid(srvc, bpid);
  return bpid;
}

int upk_buddy_start(
  upk_srvc_t    srvc,                    
  const char    *command,
  const char    *env[],
  const char    *socket

) {
  return upk_buddy_start_1(srvc,command,env,0,socket);
}

/* 
 * Stops a previously launched buddied application process and updates
 * the upkeeper runtable information.
 */
  int
upk_buddy_stop( upk_srvc_t srvc )
{
  if (upk_db_service_desired_status(
                               srvc,
                               UPK_STATUS_VALUE_STOP)) { 
    return 0;
  }
  
  return 1;
}


