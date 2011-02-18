/* 
 * upkeeper control program 
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "api/upk_api.h"
#include "store/upk_db.h"
#include "controller/controller.h"

void help( );

int DEBUG           = 0;
int OPT_START,
    OPT_STOP,
    OPT_HELP,
    OPT_VERBOSE,
    OPT_NONE
    = 0;

/* 
 * Parse command line options and set static global variables accordingly
 */
int options_parse(
    int   argc,
    char *argv[]
) {
    int c;
    int option_index;
    int nof_options = 0;

    static struct option long_options[] = {
        { "verbose",     0, &OPT_VERBOSE,   1 },
        { "start",       0, &OPT_START,   1 },
        { "stop",        0, &OPT_STOP,   1 },
        { "help",        0, &OPT_HELP, 1 },
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

	nof_options++;
    }

    if( OPT_HELP ) {
	help();
	exit( 0 );
    }

    help();

    return nof_options;
}

/*
 * Print Help
 */
void help( ) {
    printf("usage: upk [options]\n\n");

    printf(" --help:     Print this help page\n");
    printf(" --start:    Start a service\n");
    printf(" --stop:     Stop a service\n");
    printf(" --verbose:  Be verbose\n");
}

int main( 
    int   argc, 
    char *argv[] 
) {
    int      nof_options;

    nof_options = options_parse( argc, argv );

    return(0);
}
