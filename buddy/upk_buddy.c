#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include "common/sig.h"
#include "common/iopause.h"
#include "common/tai.h"
#include "common/taia.h"
#include "common/error.h"
#include "common/wait.h"
#include "common/strerr.h"

#include "store/upk_db.h"
#include "upk_buddy.h"
#include "sigblock.h"

#define FATAL "buddy: fatal: "
#define WARNING "buddy: warning: "

extern int DEBUG;

static int sigchld  = 0;
static int sigterm  = 0;
static int selfpipe[2] = {};
struct shutdown_step {
  int signal;
  int wait;
};

static void handler(int sig)  { 
  char sigc = sig;
  sig = write(selfpipe[1],&sigc,1); 
} 


static struct shutdown_step *
get_shutdown(const char *service, const char *package) 
{ 
  static struct shutdown_step s[3];
  s[0].signal = sig_term;
  s[0].wait   = 15;
  s[1].signal = sig_kill;
  s[1].wait   = 5;
  s[2].signal = -1;
  s[2].wait   = -1;
  return s;
}



void idle (sqlite3 *db, 
           int      childpid,
     const char    *package,
     const char    *service
) {

  struct taia now;
  struct taia deadline;
  iopause_fd x;
  char sig;
  int status, pid;
  struct shutdown_step *sd_config = (struct shutdown_step *)0;
  for (;;) {
    taia_uint(&deadline,3600);

    if (sd_config && sd_config->wait != -1) {
      kill(childpid,sd_config->signal);
      taia_uint(&deadline,sd_config->wait);
      sd_config++;
    }

    x.fd = selfpipe[0];
    x.events = IOPAUSE_READ;
    taia_now(&now);

    taia_add(&deadline,&now,&deadline);
    iopause(&x,1,&deadline,&now);

    sig_block(sig_child);
    sig_block(sig_term);
    if (read(selfpipe[0],&sig,1) > 0) {
      if (sig == sig_term && !sd_config) {
        sd_config = get_shutdown(package,service);
      }
    }

    pid = wait_nohang();
    if (pid) {
      upk_db_service_actual_status(db, package, service, 
                                   UPK_STATUS_VALUE_STOP);
      break;
    }

    sig_unblock(sig_child);
    sig_unblock(sig_term);

  }
  _exit (0);
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
  int ppipe[2];
  int pid;
  int w;
  char f;
  if (pipe(ppipe) == -1) {
    strerr_warn4(WARNING," pipe for ",service," failed : ",&strerr_sys);
    return;

  }

  if ((pid = fork())) {
    close(ppipe[1]);
    if (read(ppipe[0],&f,1)==-1) { }
    if (f == 'd') { return -1; }
    upk_db_service_run(
                       pdb, package, service, command, pid);
    close(ppipe[1]);
    return pid;
  }
  close(ppipe[0]);

  pid = fork();
  if (-1 == pid)  {
    w = write(ppipe[1],"fork failed",1);
    exit(123);
  }
  if (pid == 0) {
    execle(command, command, NULL, env);
    w = write(ppipe[1],"exec failed",1);
    exit(123);
  }
  if (pipe(selfpipe) == -1) { 
    w = write(ppipe[1],"pipe failed",1);
    exit(123);
  }
  ndelay_on(selfpipe[0]);
  ndelay_on(selfpipe[1]);
  sig_catch(sig_child,handler);
  sig_catch(sig_term,handler);
  w = write(ppipe[1],"u",1);
  close(ppipe[1]);
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
               const char *package,
               const char *service
               )
{
  if (upk_db_service_desired_status(
                               pdb, package, service, 
                               UPK_STATUS_VALUE_STOP)) { 
    wait(NULL);
    return 0;
 }
  
  return 1;
}

