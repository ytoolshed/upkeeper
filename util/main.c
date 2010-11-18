#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <store/upk_db.h>

int DEBUG = 0;

/*
 *
 */
void usage(
    char *prog,
    char *message
) {
    printf( "%s\n", message );
    printf( "usage: %s sqlite.db\n", prog );
    exit( 1 );
}

/*
 *
 */
int main( 
    int   argc, 
    char *argv[] 
) {
    char            *file;
    int              rc;

    if( argc != 2 ) {
	usage(argv[0], "Not enough arguments");
	exit( 1 );
    }

    file = argv[1];

    return( 0 );
}
