#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include "controller.h"
#include "common/coe.h"
#include "common/err.h"
#include "store/upk_db.h"
#include "buddy/upk_buddy.h"

/*


so, the controller starts up and it monitors two things -- a directory
and the database.


to monitor a directory, every 30 seconds, it iterates the contents of the directory
look for files of the form .buddy/%d. it attempts to connect to each of the domain sockets.

if it finds a file like this which is not a domain socket, it complains.
if it finds a domain socket it cannot connect to, it complains and removes the socket.

having connected to the socket, it sends the 's' command (status). 

if it receives a message from a buddy that it can't account for, it responds with a 't' command (terminate).

now, to start things up, the controller spins off a buddy, and adds the buddy's pid to the procruns table.


there's several messages that the controller can receive from a 

if a message is received from an unknown buddy (aka one without a procruns entry), 
the controller responds with a 'terminate' command.

'up' - the buddy has brought up the application it's monitoring. the actual state is marked as 'up'.

'down' - the application is down. the actual state is marked as 'down'.


at some point the controller should watch out for unresponsive buddies.


*/

extern int DEBUG;

struct srvc_fd { 
  /* file descriptor open to buddy, or -1 if no file descriptor yet */
  int fd;
  /* buddy pid, or -1 if this entry is unused */
  int bpid;
  /* the service that this buddy runs on */
  struct upk_srvc srvc;
};

#define UPKCTL_QUEUE_LENGTH 51
struct upkctl_event_queue {
  int pids[UPKCTL_QUEUE_LENGTH];
  int bpids[UPKCTL_QUEUE_LENGTH];
  int stats[UPKCTL_QUEUE_LENGTH];
  int idx;
};

struct upk_controller_state {
  struct upk_db  *db;
  struct srvc_fd sfd[MAX_SERVICES];
  int             service_count;
  struct upkctl_event_queue eq;
};

upkctl_t upk_controller_init(struct upk_db *db) 
{
  int i;
  upkctl_t ctl = malloc(sizeof(struct upk_controller_state));
  if (!ctl) return ctl;

  ctl->service_count=0;
  ctl->db = db;
  ctl->eq.idx = 0;

  for (i = 0; i < MAX_SERVICES; i++) {
    ctl->sfd[i].fd      = -1; 
    ctl->sfd[i].srvc.service = NULL;
    ctl->sfd[i].srvc.package = NULL;
    ctl->sfd[i].bpid    = -1;
  }

  return ctl;
}

void upk_controller_free(upkctl_t ctl) {
  int i;
  for (i = 0; i < MAX_SERVICES; i++) {
    if (ctl->sfd[i].srvc.service != NULL) { free(ctl->sfd[i].srvc.service); }
    if (ctl->sfd[i].srvc.package != NULL) { free(ctl->sfd[i].srvc.package); }
  }
  free(ctl);
}
struct srvc_fd *upkctl_find_service(upkctl_t state, upk_srvc_t srvc) {
  struct srvc_fd *limit = state->sfd + state->service_count;
  struct srvc_fd *sfd = state->sfd;

  do {
    if (sfd->srvc.service && 
        sfd->srvc.package &&
        !strcmp(srvc->service,sfd->srvc.service) &&
        !strcmp(srvc->package,sfd->srvc.package)) {
      return sfd;
    } 
  } while (++sfd < limit);
  return NULL;
}

struct srvc_fd *upkctl_make_service(upkctl_t state, upk_srvc_t srvc) {
  struct srvc_fd *limit = state->sfd + state->service_count;
  struct srvc_fd *sfd = state->sfd;
  do {
    if (sfd->srvc.service == NULL) {
      break;
    }
  } while (++sfd < limit);
  if (sfd == limit) return NULL;
  sfd->srvc.service = strdup(srvc->service);
  sfd->srvc.package = strdup(srvc->package);
  sfd->srvc.upk_db.pdb      = srvc->upk_db.pdb;
  sfd->srvc.upk_db.pdb_misc = srvc->upk_db.pdb_misc;
  return sfd;
}


int upkctl_flush_events(upkctl_t ctl) {
  int retval = 0;
  if (ctl->eq.idx != 0) {
    ctl->eq.idx = 0;
    retval = upk_db_update_buddy_events(ctl->db, 
                                        ctl->eq.bpids,
                                        ctl->eq.pids,
                                        ctl->eq.stats);
    ctl->eq.bpids[0] = 0;
    ctl->eq.pids[0]  = 0;
    ctl->eq.stats[0] = 0;
  }
  return retval;

}
                                
int upkctl_handle_buddy_status(
                               upkctl_t state,
                               int  sock,
                               char *msg
                               ) {
  int bpid,pid,status;
  memcpy(&bpid,   msg+1,sizeof(int));
  memcpy(&pid,    msg+1+sizeof(int),sizeof(int));
  memcpy(&status, msg+1+(sizeof(int)*2),sizeof(int));
  switch (msg[0]) {
  case 'u': /* message from buddy */
    if ( DEBUG ) {
      printf("BUDDY UP MESSAGE bpid: %d, pid: %d\n", bpid, pid);
    }
    state->eq.bpids[state->eq.idx] = bpid;
    state->eq.pids[state->eq.idx]  = pid;
    state->eq.idx++;
    if (state->eq.idx == 50) {
      return upkctl_flush_events( state );
    }
    break;
  case 'd': /* message from buddy */
    if ( DEBUG ) {
      printf("BUDDY DOWN MESSAGE bpid: %d, pid: %d\n", bpid, pid);
    }
    state->eq.bpids[state->eq.idx] = bpid;
    state->eq.pids[state->eq.idx]  = 0;
    state->eq.stats[state->eq.idx] = status;
    state->eq.idx++;
    if (state->eq.idx == 50) {
      return upkctl_flush_events( state );
    }
    break;
  case 'e': /* message from buddy */
    /* XXX: can happen out of order due to immeadiate handlign */
    if ( DEBUG ) {
      printf("BUDDY EXITED MESSAGE bpid: %d, pid: %d\n", bpid, pid);
    }
    upk_db_buddy_down(state->db, bpid);
    break;
  default:
    break;
  }

   return 1;
}


void upkctl_stop_service(upkctl_t state, upk_srvc_t srvc) {

}

void upkctl_start_service(upkctl_t state, upk_srvc_t srvc) {

}


/* 
 * Bring a service inline with the desired status
 */
void upkctl_status_fixer_callback(
                                          void * context,
                                          upk_srvc_t  srvc,     
                                          const char    *status_desired,
                                          const char    *status_actual
                                          ) {
    upkctl_t ctl = context;
    if (strcmp(status_desired, upk_states[UPK_STATE_STOP])) {
      upkctl_stop_service(ctl, srvc);
    } else {
      upkctl_start_service(ctl, srvc);
    }
    return;
}


/* 
 * Bring all services inline with the desired status
 */
void upkctl_status_fixer(upkctl_t ctl) 
{
  upk_db_status_visitor(ctl->db->pdb, upkctl_status_fixer_callback, (void *)ctl);
}

void
upkctl_build_fdset(upkctl_t state, fd_set *rfds, int *maxfd) {
  struct srvc_fd *sfd;
  for (sfd = state->sfd; sfd < state->sfd + MAX_SERVICES; sfd++) {
    if ( !sfd->srvc.service ) continue;
    if ( sfd->fd == -1 )      continue;
    if ( sfd->fd > *maxfd ) *maxfd = sfd->fd;
    FD_SET(sfd->fd, rfds); 
  }
}

/* 
to monitor a directory, every 30 seconds, it iterates the contents of the directory
look for files of the form .buddy/%d. it attempts to connect to each of the domain sockets.

if it finds a file like this which is not a domain socket, it complains.
if it finds a domain socket it cannot connect to, it complains and removes the socket.

having connected to the socket, it sends the 's' command (status). 
*/

int 
upkctl_scan_directory(upkctl_t state) {
  DIR *dir;
  struct dirent *de;
  dir = opendir(".");
  if (!dir) {
    return -1;
  }
  while (de = readdir(dir)) {
    if (de->d_name[0] == '.' &&
        de->d_name[1] == 'b' &&
        de->d_name[2] == 'u' &&
        de->d_name[3] == 'd' &&
        de->d_name[4] == 'd' &&
        de->d_name[5] == 'y') {
      /* found a buddy -- make sure it's a domain socket */
      struct stat buf;
      if (stat(de->d_name,&buf) == -1) {
        continue; /*xxx*/
      }
      if (! S_ISSOCK(buf.st_mode)) {
        continue; /*xxx*/
      }
      /*     sfd->fd = upk_buddy_connect(sfd->bpid); */

    }
  }
}


static void upkctl_service_callback(void *context,
				   upk_srvc_t srvc,
				   const char *ignored,
				   const char *ignored_also)
{
  /* build some tables */
}
int upkctl_sync_services(upkctl_t state) 
{
  upk_db_status_visitor(state->db,upkctl_service_callback, state);
}
