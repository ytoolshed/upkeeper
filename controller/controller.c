#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include "controller.h"
#include "common/coe.h"
#include "common/err.h"
#include "store/upk_db.h"
#include "buddy/upk_buddy.h"
extern int DEBUG;

struct status_fixer_state {
  sqlite3 *pdb;
  struct srvc_fd *sfd;
  int    service_count;
};

struct srvc_fd *find_service(struct srvc_fd *sfd, upk_srvc_t srvc, int srvcount) {
  struct srvc_fd *limit = sfd + srvcount;
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

struct srvc_fd *make_service(struct srvc_fd *sfd, upk_srvc_t srvc, int srvcount) {
  struct srvc_fd *limit = sfd + srvcount;
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

/* 
 * Bring a service inline with the desired status
 */
void upk_controller_status_fixer_callback( 
    void *context,                                          
    upk_srvc_t  srvc,                                    
    char    *status_desired,
    char    *status_actual
) {
    struct status_fixer_state *s = context;
    const char *cmdline;
    char       *cmdline_alloc;
    struct     srvc_fd *sfd = find_service(s->sfd, srvc, s->service_count);
    int        buddyfd;

    if (sfd == NULL) { /* we have no fd for this service */
      int bpid = upk_db_service_buddy_pid(srvc, 0);
      if (bpid) {
        if ((buddyfd = upk_buddy_connect(bpid)) == -1) {
          if (errno == ENOENT || errno == ECONNREFUSED) {
            upk_db_buddy_down(s->pdb, bpid);
            upk_db_service_actual_status(srvc,UPK_STATE_STOP);
            status_actual = "stop";
          } 
        } else {
          printf("*** FOUND buddy pid in db : %d\n",bpid);
          sfd = make_service(s->sfd, srvc, s->service_count);
          sfd->bpid = bpid;
          sfd->fd   = buddyfd;
          if (write(sfd->fd,"s",1) != 1) {
            /* XXX*/
          }
          upk_db_service_actual_status(srvc,UPK_STATE_START);
          status_actual = "start";
        }
      }
    }

    if( strcmp( status_desired, status_actual ) == 0 ) {
      return ;
    }

    cmdline = upk_db_service_cmdline( srvc, NULL );

    if( cmdline == NULL ) {
        if( DEBUG ) {
	    printf("** ERROR no cmdline for %s/%s\n", 
                   srvc->package, srvc->service );
        }
        return;
    }

      /* We got a static buffer which is going to be overridden with the
         next sql call, allocate memory */
    cmdline_alloc = strdup( cmdline );

    if( strcmp( status_desired, upk_states[ UPK_STATE_START ] ) == 0 ) {
        if (sfd && sfd->fd == -1) {
            sfd->fd = upk_buddy_connect(sfd->bpid);
            if (sfd->fd == -1) {
                printf("*** WAITING for buddy %s/%s/%d\n", 
                       srvc->package,srvc->service, errno);
                return;
            }
            printf("*** FOUND buddy %s/%s/%d\n", 
                   srvc->package,srvc->service, sfd->bpid);
        
            if (write(sfd->fd,"s",1) != 1) {
                /* XXX*/
            }
            return;
            

        } 
        
	/* service needs to be started */
        sfd = make_service(s->sfd, srvc, s->service_count);
        if( 1 ) {
          printf("** STARTING %s/%s/%s\n", srvc->package, srvc->service,
                   cmdline_alloc );
        }
        sfd->bpid = upk_buddy_start( srvc, cmdline_alloc, NULL );
        upk_db_service_buddy_pid(srvc, sfd->bpid);
        sfd->fd   = -1;
    } else {
        if( 1 ) {
	    printf("** STOPPING %s/%s/%s\n", srvc->package, srvc->service,
                                             cmdline_alloc );
        }
        upk_service_buddy_stop( srvc );
    }
    
    free( cmdline_alloc );
    return;
}

/* 
 * Bring all services inline with the desired status
 */
void upk_controller_status_fixer( 
                                 sqlite3 *pdb,
                                 struct srvc_fd     *fd
) {
  struct status_fixer_state s = { pdb, fd, MAX_SERVICES };
  upk_db_status_visitor( pdb, upk_controller_status_fixer_callback, (void *)&s);
}


static int bpids[51];
static int  pids[51];
static int stats[51];
static int  idx = 0;

int upk_controller_flush_events(
    struct upk_db *upk_db
) {
  int retval = 0;
  if (idx != 0) {
    idx = 0;
    retval = upk_db_update_buddy_events(upk_db, bpids,pids,stats);
    bpids[0] = 0;
    pids[0]  = 0;
    stats[0] = 0;
  }
  return retval;

}
                                
int upk_controller_handle_buddy_status(
    struct upk_db *upk_db,
    int  sock,
    char *msg
) {
  int bpid,pid,status;
  int ret;
  memcpy(&bpid,   msg+1,sizeof(int));
  memcpy(&pid,    msg+1+sizeof(int),sizeof(int));
  memcpy(&status, msg+1+(sizeof(int)*2),sizeof(int));
  switch (msg[0]) {
  case 'u': /* message from buddy */
    if ( DEBUG ) {
      printf("BUDDY UP MESSAGE bpid: %d, pid: %d\n", bpid, pid);
    }
    bpids[idx] = bpid;
    pids[idx] =   pid;
    idx++;
    if (idx == 50) {
      return upk_controller_flush_events( upk_db );
    }
    break;
  case 'd': /* message from buddy */
    if ( DEBUG ) {
      printf("BUDDY DOWN MESSAGE bpid: %d, pid: %d\n", bpid, pid);
    }
    bpids[idx] = bpid;
     pids[idx] = 0;
    stats[idx] = status;
    idx++;
    if (idx == 50) {
      return upk_controller_flush_events( upk_db );
    }
    break;
  case 'e': /* message from buddy */
    if ( DEBUG ) {
      printf("BUDDY EXITED MESSAGE bpid: %d, pid: %d\n", bpid, pid);
    }
    upk_db_buddy_down(upk_db->pdb, bpid);
    break;
  default:
    break;
  }

   return 1;
}
