#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "../store/upk_db.h"
#include <unistd.h>
#include "controller.h"

int DEBUG = 0;

static int OPT_BOOTSTRAP        = 0;
static int OPT_VERBOSE          = 0;
static int OPT_SIGNAL_LISTENERS = 0;
static int OPT_STATUS_FIXER     = 1;

int main( 
    int   argc, 
    char *argv[] 
) {
    sqlite3 *pdb;
    char    *file = "../store/store.sqlite";
    int      rc;
    int      opt;
    
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

    if( OPT_STATUS_FIXER ) {
      upk_controller_status_fixer( pdb, file);
    }

    if( OPT_SIGNAL_LISTENERS ) {
	/* Just signal all registered listeners and exit */
        upk_db_listener_send_all_signals( pdb );
	goto SHUTDOWN;
    }


    /* Notify all listeners */
    upk_db_listener_send_all_signals( pdb );

SHUTDOWN:
    upk_db_close( pdb );

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
        { "status-fixer", 0, &OPT_STATUS_FIXER, 1 },
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
