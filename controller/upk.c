#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "../store/upk_db.h"

int DEBUG        = 0;
int OPT_ALL_UP   = 0;
int OPT_ALL_DOWN = 0;

int main( 
    int   argc, 
    char *argv[] 
) {
    sqlite3 *pdb;
    char    *file = "../store/store.sqlite";
    int      rc;
    int      i;
    char    *service;
    char    *package;
    const char    *cp;

    options_parse( argc, argv );

    rc = upk_db_init( file, &pdb );

    if(rc < 0) {
	printf("upk_db_init failed. Exiting.\n");
	exit(-1);
    }
 
    if( OPT_ALL_UP || OPT_ALL_DOWN ) {

	char *status = "start";
	if( OPT_ALL_DOWN ) 
	    status = "stop";

        for( i=0; i<=5; i++ ) {
            service = sqlite3_mprintf("service-%d", i);
            package = sqlite3_mprintf("package-%d", i);

            upk_db_service_actual_status( pdb, package, service, status);

            sqlite3_free( service );
            sqlite3_free( package );
        }
    }

    upk_db_listener_send_all_signals( pdb );

    sqlite3_close( pdb );

    return(0);
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
        { "all-up",   0, &OPT_ALL_UP,   1 },
        { "all-down", 0, &OPT_ALL_DOWN, 1 },
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
