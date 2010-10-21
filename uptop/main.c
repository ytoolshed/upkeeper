#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "../store/upk_db.h"
#include <unistd.h>
#include "uptop.h"

int DEBUG = 0;

static OPT_VERSION = 0;

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

    uptop_services_print( pdb );

    upk_db_close( pdb );

    return(0);
}

/* 
 * Status iterator callback to reset all states to 'stop'.
 */
void uptop_print_callback( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service,
    char    *status_desired,
    char    *status_actual
) {

    char *fq_service;

    if( status_actual == NULL ) {
	status_actual = "UNDEF";
    }

    fq_service = sqlite3_mprintf(
	    "%s-%s", package, service);

    printf( "%-20s: %-5s [%-5s]\n", fq_service, status_actual, status_desired );

    sqlite3_free( fq_service );
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
        { "version", 0, &OPT_VERSION, 1 },
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

/* 
 * Print the status of all services
 */
void uptop_services_print(
    sqlite3 *pdb
) {
    upk_db_status_checker( pdb, uptop_print_callback );
}

