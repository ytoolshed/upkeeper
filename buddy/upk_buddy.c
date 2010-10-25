#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include "common/sig.h"
#include "common/iopause.h"
#include "common/tai.h"
#include "common/taia.h"
#include "common/coe.h"
#include "common/error.h"
#include "common/wait.h"
#include "common/strerr.h"

#include "store/upk_db.h"
#include "upk_buddy.h"
#define BUDDYPATH "./buddy"

/* 
 * Launches the application process in argv[] with a 'buddy'
 * which updates the upkeeper database when the application has been started
 * and when it got stopped or crashed.
 */

int upk_buddy_start(
  upk_srvc_t    srvc,                    
  const char    *command,
  const char    *env[]
) {
  int ppipe[2];
  int pid,bpid;
  int wstat;
  int w;
  
  /* set up communication path */
  if (pipe(ppipe) == -1) {
    /*strerr_warn4(WARNING," pipe for ",srvc->service," failed : ",&strerr_sys);*/
    return -1;

  }
  coe(ppipe[0]);

  bpid = fork();
  if (bpid == -1) return -1;

  if (bpid == 0) {
    if (fd_copy(3, ppipe[1]) == -1)
      close(ppipe[1]);

    execle(BUDDYPATH, 
           BUDDYPATH, 
           command, 
           "........................................",
           NULL,
           env);
    close(ppipe[1]);
    exit(1);
  }

  close(ppipe[1]);
  if (read(ppipe[0],&pid,4) == 4) { 
    upk_db_service_run(srvc, command, pid);      
    return pid;
  }

  close(ppipe[0]);
  return pid;
  
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
    wait(NULL);
    return 0;
  }
  
  return 1;
}

