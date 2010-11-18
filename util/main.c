#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <store/upk_db.h>

int DEBUG = 0;

int main( 
    int   argc, 
    char *argv[] 
) {
    char            *file = "store.sqlite";
    int              rc;
    int              i;
    const char      *cp;
    sqlite3         *pdb;

    rc = upk_db_init( file, &pdb );
    
    if( rc < 0 ) {
        printf( "init rc=%d\n", rc );
	exit( 1 );
    }

    upk_db_exec_single( pdb, ".dump" );

    sqlite3_close( pdb );

    return(0);
}
