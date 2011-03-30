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
#include <assert.h>

#define STRING_OR_NULL( x ) ( (x) ? ( x ) : "[unset]" )

void help( );

int DEBUG           = 0;
int OPT_START,
    OPT_STOP,
    OPT_HELP,
    OPT_LIST,
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
        { "list",        0, &OPT_LIST,   1 },
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

    return nof_options;
}

/*
 * Print Help
 */
void help( ) {
    printf("usage: upk [options]\n\n");

    printf(" --help:     Print this help page\n");
    printf(" --list:     List all services\n");
    printf(" --start:    Start a service\n");
    printf(" --stop:     Stop a service\n");
    printf(" --verbose:  Be verbose\n");
}

void service_list_callback(
   struct upk_api_service *data,
   void  *context
) {
    printf( "%s/%s %s %s %s %d\n", 
	    STRING_OR_NULL( data->package ), 
            STRING_OR_NULL( data->service ), 
	    STRING_OR_NULL( data->cmdline ), 
            STRING_OR_NULL( data->state_actual ), 
	    STRING_OR_NULL( data->state_desired ),
            data->pid
	  );
}

int main(
    int   argc, 
    char *argv[] 
) {
    int             nof_options;
    int             rc;
    struct upk_api  upk_api;

    nof_options = options_parse( argc, argv );

    upk_db_file_main_set( "../store/upkeeper-main.sqlite" );
    upk_db_file_misc_set( "../store/upkeeper-misc.sqlite" );

    assert( upk_api_init( &upk_api ) == 0 );

    if( 0 ) {
    } else if( OPT_LIST ) {
        upk_api_service_visitor( &upk_api, service_list_callback, NULL );
    } else {
	help();
    }

    return(0);
}
