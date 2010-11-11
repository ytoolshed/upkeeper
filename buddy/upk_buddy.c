#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>

#include "store/upk_db.h"
#include "upk_buddy.h"
#define BUDDYPATH "../buddy"

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
  int async
) {
  int   bpid;
  int   p[2]; 
  char  c;

  if( DEBUG ) {
      printf("upk_buddy_start '%s' '%s' [%s] async=%d\n",
              srvc->service, srvc->package, command, async);
  }

  if (!async && pipe(p) == -1) return -1;
  coe(p[0]); coe(p[1]);
  bpid = fork();
  
  if (bpid == -1) { 
    close(p[0]); close(p[1]);
    return -1;
  }

  if (bpid == 0) {
    if (!async && !dup2(p[1],3)) {
        exit(111);
    }
    execle(BUDDYPATH, 
           BUDDYPATH, 
           command, 
           "........................................",
           "-s", srvc->service,
           "-p", srvc->package,
           "-d", "./store.sqlite",
           "-f",
           NULL,
           env);
    exit(112);
  }
  close(p[1]);
  if (!async && read(p[0],&c, 1) != 1){
    return -1;
  }
  close(p[0]);
  return bpid;
}

int upk_buddy_start(
  upk_srvc_t    srvc,                    
  const char    *command,
  const char    *env[]

) {
  return upk_buddy_start_1(srvc,command,env,0);
}

int upk_buddy_start_async(
  upk_srvc_t    srvc,                    
  const char    *command,
  const char    *env[]

) {
  return upk_buddy_start_1(srvc,command,env,1);
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


