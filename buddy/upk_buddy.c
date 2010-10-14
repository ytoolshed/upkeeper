#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "store/upk_db.h"
#include "upk_buddy.h"
#include "sigblock.h"
extern int DEBUG;

static int sigchld  = 0;
static int sigterm  = 0;
struct shutdown_step {
  int signal;
  int wait;
};

static void sigchld_handler(int ignored)  { sigchld = 1; } 
static void sigterm_handler(int ignored)  { sigterm = 1; } 


struct shutdown_step steps[3] = 
  { { SIGTERM, 15 }, 
    { SIGKILL,  5 },
    { -1,      -1}};

static struct shutdown_step *
get_shutdown(const char *service, const char *package) 
{ 
  return steps;
}


void idle (sqlite3 *db, 
           int      childpid,
     const char    *package,
     const char    *service
) {
  int status, pid;
  struct shutdown_step *sd_config = NULL;
  for (;;) {
    if (sd_config && sd_config->wait) {
      if (sd_config->wait == -1) break;
      sleep(sd_config->wait);
      sd_config++;
    } else {
      pause();
    }

    upk_block_signal(SIGCHLD);
    upk_block_signal(SIGTERM);
    if (sigchld) {
      pid = waitpid(childpid,&status,WNOHANG);
      if       (0 == pid) { printf("got a sigchild but waitpid returns 0!?"); }
      else if (-1 == pid) { printf("waitpid failed !? :%s\n",strerror(errno)); }
      else                { upk_db_service_actual_status(db,package, service, 
                                                         UPK_STATUS_VALUE_STOP); }
      sigchld = 0;
    } else if (sigterm && !sd_config) {
      sd_config = get_shutdown(package,service);
    }
    sigterm = sigchld = 0;
    upk_unblock_signal(SIGCHLD);
    upk_unblock_signal(SIGTERM);

  }
  exit (0);
}

/* 
 * Launches the application process in argv[] with a 'buddy'
 * which updates the upkeeper database when the application has been started
 * and when it got stopped or crashed.
 */

int upk_buddy_start(
  sqlite3 *pdb, 
  const char    *package,
  const char    *service,
  const char    *command,
  const char    *env[]
) {
  int pid;
  if ((pid = fork())) {
    return pid;
  }
  upk_catch_signal(SIGCHLD,sigchld_handler);
  upk_catch_signal(SIGCHLD,sigterm_handler);
  upk_block_signal(SIGCHLD);
  
  pid = fork();
  if (-1 == pid)  {
    upk_db_service_actual_status(pdb, package, service, UPK_STATUS_VALUE_STOP);
    exit(123);
  }

  if (pid == 0) {
    execle(command, command, NULL, env);
    exit(123);
  }

  upk_db_service_actual_status(pdb, package, service, UPK_STATUS_VALUE_START);   
  upk_unblock_signal(SIGCHLD);
  idle(pdb, pid, package, service);
  exit(0);
}



/* 
 * Stops a previously launched buddied application process and updates
 * the upkeeper runtable information.
 */
int
upk_buddy_stop(
               sqlite3 *pdb,
  int       buddy_pid
) 
{
    return kill(SIGTERM, buddy_pid);
}
