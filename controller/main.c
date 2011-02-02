#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include "../store/upk_db.h"
#include "controller.h"
#include "common/coe.h"
#include "common/err.h"
#include "common/nonblock.h"
#include "common/sigblock.h"
#include "buddy/upk_buddy.h"

int DEBUG = 0;

static int OPT_VERBOSE          = 0;

static int term  = 0;
static int hup   = 0;
static int needs_flush = 0;
static int sigp[2]            = {-1,-1};
/* 
 * Parse command line options and set static global variables accordingly
 */
int options_parse(
    int   argc,
    char *argv[]
) {
    int c;
    int option_index;
    static struct option long_options[] = {
        { "verbose", 0, &OPT_VERBOSE, 1 },
	{ 0, 0, 0, 0 }
    };

    if( DEBUG ) {
        printf( "Parsing command line options\n" );
    }

    while (1) {

        c = getopt_long (argc, argv, "",
                         long_options, &option_index);
        if( c == -1 ) {
            break;
        }
    }
    return option_index;
}

#define LISTEN_PATH "./controller"


static void handler(int sig)  
{ 
  switch (sig) {
  case SIGTERM: term = 1; break;
  case SIGHUP:  hup  = 1; break;
  default: break;
  }
  sig = write(sigp[1]," ",1);
}


int main( 
    int   argc, 
    char *argv[] 
) {
    sqlite3 *pdb;
    char    *file = "../store/store.sqlite";
    int      rc;
    int      sock;
    struct srvc_fd fds[MAX_SERVICES] = {  };
    struct srvc_fd *sfd = fds;


    options_parse( argc, argv );
    if (optind < argc) {
      file = argv[optind];
    }

    if(OPT_VERBOSE) {
	printf("verbose output enabled\n");
	DEBUG = 1;
    }

    rc = upk_db_init( file, &pdb );

    if(rc < 0) {
        sysdie3(111, FATAL, "failed to initialize database ","");
	exit(-1);
    }

    for (sfd = fds ; sfd < fds + MAX_SERVICES; sfd++) {
      sfd->fd = -1;
      sfd->srvc.service = NULL;
      sfd->srvc.package = NULL;
      sfd->bpid         = -1;
    }

    if (pipe(sigp) == -1)
      sysdie3(111,FATAL, "failed to create pipe for signals ","");

    coe(sigp[0]); nonblock(sigp[0]);
    coe(sigp[1]); nonblock(sigp[1]);
    
    upk_catch_signal(SIGTERM, handler);
    upk_catch_signal(SIGHUP,  handler);
    upk_controller_status_fixer( pdb, fds);

    upk_db_listener_remove_dead( pdb ); 
    upk_db_listener_add( pdb, "controller", getpid(), SIGHUP );

    for (;;) {
      struct timeval period;
      int maxfd = sigp[0];
      fd_set rfds;
      char sig;
      int need_notify;
      char mbuf[128];
      FD_ZERO(&rfds);
      FD_SET(sigp[0], &rfds);
      for (sfd = fds ; sfd < fds + MAX_SERVICES; sfd++) {
        if ( !sfd->srvc.service ) continue;
        if ( sfd->fd == -1)       continue ;
        FD_SET(sfd->fd, &rfds); 
        if (sfd->fd > maxfd) maxfd = sfd->fd;
      }
      period.tv_sec  = 1;
      period.tv_usec = 0;

      switch(select(maxfd+1, &rfds, NULL, NULL, &period)) {
      case -1:
        FD_ZERO(&rfds);
	break;
      case 0:
	needs_flush = 1;
	break;
      default:
	break;
      }
      
      while(read(sigp[0],&sig,1) > 0) 
        ;

      for (sfd = fds ; sfd < fds + MAX_SERVICES; sfd++) {
        if ( !sfd->srvc.service ) continue;
        if ( sfd->fd == -1 ) {
          sfd->fd = upk_buddy_connect(sfd->bpid);
          if (sfd->fd == -1) continue;
          nonblock(sfd->fd);
          if (write(sfd->fd,"s",1) != 1) {
            syswarn3("writing status request to buddy for ",sfd->srvc.service," failed: ");
            close(sfd->fd); sfd->fd == -1;
          }
          continue;
        } 
        if ( FD_ISSET(sfd->fd,&rfds) ) {
          while (read(sfd->fd,mbuf,sizeof(int)*3 + 1) > 0) {
            if (!upk_controller_handle_buddy_status(sfd->srvc.pdb,
                                                    sfd->fd,
                                                    mbuf)) {
              need_notify = 1;
            }
          }
        }
      }
      if (need_notify) {
        need_notify = 0;
        upk_db_listener_send_all_signals( pdb );
      }
      if (term) {
        break;
      }
      if (hup) {
        hup = 0;
        upk_controller_status_fixer( pdb, fds);
      }
      if (needs_flush) {
	upk_controller_flush_events( pdb);
	needs_flush = 0;
      }
      
    }
    
    upk_db_close( pdb );
    unlink(LISTEN_PATH);
    return(0);
}

/* 
 * Status iterator callback to reset all states to 'stop'.
 */
void upk_db_reset_launchcallback( 
    upk_srvc_t  srvc,                                    
    char    *status_desired,
    char    *status_actual
) {
  if( status_actual != NULL &&
      strcmp( status_actual, "start" ) == 0 ) {
    if( DEBUG )
      printf( "Resetting status of service %s-%s.\n", 
	      srvc->package, srvc->service );
   
    upk_db_service_actual_status( srvc, UPK_STATE_STOP );
  }
}

/* 
 * Reset all services during bootstrap.
 */
void upk_controller_bootstrap( 
    sqlite3 *pdb
) {
  upk_db_status_visitor( pdb, upk_db_reset_launchcallback, NULL );
}

