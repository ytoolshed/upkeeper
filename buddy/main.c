#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "store/upk_db.h"
#include "upk_buddy.h"

int DEBUG = 0;

int main( 
    int   argc, 
    char *argv[] 
) {
    sqlite3 *pdb;
    char    *file = "../store/store.sqlite";
    int      rc;
    int      i;
    int      pid;
    char    *event;
    char    *status;
    char    *service;
    char    *package;
    char    *cp;
    const char    *command = "./do-sleep";

    rc = upk_db_init( file, &pdb );

    if(rc < 0) {
	printf("db_init failed. Exiting.\n");
	exit(-1);
    }
 
    pid = upk_buddy_start( pdb, "package-1", "service-1", command, NULL);

    if( pid < 0 ) {
	printf( "upk_buddy_start failed (%d)\n", pid );
	exit( -1 );
    }

    upk_buddy_stop( pdb, pid );

    sqlite3_close( pdb );
}
