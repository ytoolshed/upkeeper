#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "../store/upk_db.h"
#include <unistd.h>
#include "controller.h"

int DEBUG = 0;

static int OPT_BOOTSTRAP        = 0;
static int OPT_SIGNAL_LISTENERS = 0;

int main( 
    int   argc, 
    char *argv[] 
) {
    sqlite3 *pdb;
    char    *file = "../store/store.sqlite";
    int      rc;

    options_parse( argc, argv );

    rc = upk_db_init( file, &pdb );

    if(rc < 0) {
	printf("db_init failed. Exiting.\n");
	exit(-1);
    }

    if( OPT_SIGNAL_LISTENERS ) {
	/* Just signal all registered listeners and exit */
        upk_db_listener_send_all_signals( pdb );
	goto SHUTDOWN;
    }

    if( OPT_BOOTSTRAP ) {
        /* In bootstrap mode, reset all process entries in the DB to 
         * "down" to start with a clean slate.
         */
	if( DEBUG )
	    printf("Bootstrap mode.\n");

	upk_controller_bootstrap( pdb );
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
    sqlite3 *pdb, 
    char    *package, 
    char    *service,
    char    *status_desired,
    char    *status_actual
) {

    if( status_actual == NULL ||
	strcmp( status_actual, "start" ) == 0 ) {
	if( DEBUG )
	    printf( "Resetting status of service %s-%s.\n", package, service );
        upk_db_service_actual_status( pdb, package, service, "stop" );
    }
}

/* 
 * Reset all services during bootstrap.
 */
void upk_controller_bootstrap( 
    sqlite3 *pdb
) {
    upk_db_status_checker( pdb, upk_db_reset_launchcallback );
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
        { "bootstrap", 0, &OPT_BOOTSTRAP, 1 },
        { "signal-listeners", 0, &OPT_SIGNAL_LISTENERS, 1 },
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
}
