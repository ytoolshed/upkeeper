#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "../store/upk_db.h"
#include <unistd.h>

int DEBUG = 1;

static OPT_BOOTSTRAP = 0;

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

    /* In bootstrap mode, reset all process entries in the DB to 
     * "down" to start with a clean slate.
     */
    if( OPT_BOOTSTRAP ) {
        /* 
         */
	if( DEBUG )
	    printf("Bootstrap mode.\n");
    }

    upk_db_close( pdb );

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
        { "bootstrap", 0, &OPT_BOOTSTRAP, 1 },
	{ 0, 0, 0, 0 }
    };

    while (1) {

	if( DEBUG ) {
	    printf( "Parsing command line options\n" );
	}

        c = getopt_long (argc, argv, "",
                         long_options, &option_index);

        if( c == -1 ) {
            break;
        }
    }
}
