#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "store/upk_db.h"
#include "upk_buddy.h"
#define BUDDYPATH "buddy"

extern int DEBUG;

/* 
 * Launches the application process in argv[] with a 'buddy'
 * which updates the upkeeper database when the application has been started
 * and when it got stopped or crashed.
 */

int upk_buddy_start_1(
  upk_srvc_t    srvc,                    
  const char    *command,
  const char    *env[]
) {
  int   bpid;

  if( DEBUG ) {
      printf("upk_buddy_start '%s' '%s' [%s]\n",
              srvc->service, srvc->package, command);
  }

  bpid = fork();
  
  if (bpid == -1) { 
    return -1;
  }

  if (bpid == 0) {
    upk_db_close(srvc->pdb);
    execlp(BUDDYPATH, 
           BUDDYPATH, 
           "-q",
           "................................................................................"
           "................................................................................"
           "................................................................................",
           command, 
           NULL);


    if( DEBUG ) {
        printf("execle %s [%s] failed (%s)\n",
                BUDDYPATH, command, strerror( errno ) );
    }

    _exit(112);
  }
  return bpid;
}

int upk_buddy_start(
  upk_srvc_t    srvc,                    
  const char    *command,
  const char    *env[]

) {
  return upk_buddy_start_1(srvc,command,env);
}

/* 
 * Stops a previously launched buddied application process and updates
 * the upkeeper runtable information.
 */
int
upk_buddy_stop( int pid )
{
  if (upk_buddy_send_message(pid, BUDDY_EXIT) == -1) {
    return 1;
  }
  return 0;
}


int
upk_service_buddy_stop( upk_srvc_t srvc )
{
  int bpid = upk_db_service_buddy_pid(srvc, 0);
  if (bpid < 0) {
    errno = EINVAL;    
    return -1;
  }
  return upk_buddy_stop(bpid);

}

char buddymsg[] = "st";

int upk_service_buddy_send_message (upk_srvc_t srvc, upk_buddy_message m) 
{
  int bpid = upk_db_service_buddy_pid(srvc, 0);
  if (bpid < 0) {
    errno = EINVAL;    
    return -1;
  }
  return upk_buddy_send_message(bpid,m);
}

int upk_service_buddy_connect (upk_srvc_t srvc)
{
  int bpid = upk_db_service_buddy_pid(srvc, 0);
  if (bpid < 0) {
    errno = EINVAL;    
    return -1;
  }
  return upk_buddy_connect(bpid);


}

int upk_buddy_connect (int bpid)
{
  int s    = socket(AF_UNIX, SOCK_STREAM, 0);
  struct sockaddr_un baddr;

  if (s == -1) {
    return -1;
  }
  baddr.sun_family = AF_UNIX;
  sprintf(baddr.sun_path,"./.buddy.%d",bpid);

  if (connect(s,(struct sockaddr *)&baddr, SUN_LEN(&baddr)) == -1) {
    int erno = errno;
    close(s);
    errno = erno;
    return -1;
  }
  
  return s;
}

int upk_buddy_send_message(int bpid, upk_buddy_message m) 
{
  int s = upk_buddy_connect(bpid);
  char msg = buddymsg[m];

  if (send(s, &msg, 1, 0) < 0) {
    close(s);
    return -1;
  }
  close(s);
  return 0;
}

