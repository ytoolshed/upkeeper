#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "../store/upk_db.h"
#include <unistd.h>
#include "controller.h"
#include "common/coe.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#define FATAL "buddy-controller: fatal: "

int DEBUG = 0;

static int OPT_BOOTSTRAP        = 0;
static int OPT_VERBOSE          = 0;
static int OPT_SIGNAL_LISTENERS = 0;
static int OPT_STATUS_FIXER     = 1;
static int OPT_SOCKET_LISTENER  = 1;

#define LISTEN_PATH "./controller"
static void sysdie(int e, const char *a,const char *b, const char *c)
{
  if (write(2, a, strlen(a))
      && write(2, b, strlen(a))
      && write(2, c, strlen(a))
      && write(2, ": ", 2)
      && write(2, strerror(errno),strlen(strerror(errno)))
      && write(2, "\n", 1)) ;
  exit(e);
}

int setup_socket (char *path) {
  struct sockaddr_un sa;
  struct stat st;
  int s;
  int yes;

  s = socket(AF_UNIX, SOCK_DGRAM, 0);

  if (s == -1) 
    sysdie(111,FATAL,"socket failed","");

  if (stat(path,&st) != -1 && (S_IFSOCK & st.st_mode)) {
    unlink(path);
  }    
  sa.sun_family = AF_UNIX;
  strcpy(sa.sun_path,path);

  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
    sysdie(111,FATAL,"socket failed","");
    exit(1);
  }
  if (bind(s, (struct sockaddr *) &sa, sizeof(struct sockaddr_un))) {
    sysdie(111,FATAL,"binding to socket","");
    exit(1);
  }
  coe(s);
  return s;
}

int main( 
    int   argc, 
    char *argv[] 
) {
    sqlite3 *pdb;
    char    *file = "../store/store.sqlite";
    int      rc;
    int      opt;
    int      sock;
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
	printf("db_init failed. Exiting.\n");
	exit(-1);
    }
    

    if( OPT_BOOTSTRAP ) {
        /* In bootstrap mode, reset all process entries in the DB to 
         * "down" to start with a clean slate.
         */
	if( DEBUG )
	    printf("Bootstrap mode.\n");

	upk_controller_bootstrap( pdb );
    }

    if ( OPT_SOCKET_LISTENER ) {
      sock = setup_socket(LISTEN_PATH);
    }

    if( OPT_STATUS_FIXER ) {
      upk_controller_status_fixer( pdb, LISTEN_PATH);
    }

    if ( OPT_SOCKET_LISTENER ) {
          char msg[1+2*sizeof(int)];

      while (read(sock,msg,sizeof(int)*2 + 1)) {
        int bpid,arg;
        memcpy(&bpid,msg+1,sizeof(int));
        memcpy(&arg, msg+1+sizeof(int),sizeof(int));
        if ( DEBUG ) {
          printf("%c buddy:%d pidorstatus:%d\n",msg[0],bpid,arg);
        }
        switch (msg[0]) {
        case 'u':
          upk_db_set_pid_for_buddy(pdb,arg,bpid);
          break;
        case 'd':
          upk_db_note_exit(pdb,arg,bpid);
          break;
        default:
          break;
        }
      }
      
    }
SHUTDOWN:
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
   
    upk_db_service_actual_status( srvc, UPK_STATUS_VALUE_STOP );
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
        { "bootstrap", 0, &OPT_BOOTSTRAP, 1 },
        { "signal-listeners", 0, &OPT_SIGNAL_LISTENERS, 1 },
        { "listen-socket", 1, &OPT_SOCKET_LISTENER, 's' },
        { "status-fixer", 0, &OPT_STATUS_FIXER, 1 },
	{ 0, 0, 0, 0 }
    };

    if( DEBUG ) {
        printf( "Parsing command line options\n" );
    }

    while (1) {

        c = getopt_long (argc, argv, "",
                         long_options, &option_index);
        if( c == 's') {
          
        }
        if( c == -1 ) {
            break;
        }
    }
    return option_index;
}
