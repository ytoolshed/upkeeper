#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "store/upk_db.h"
#include "upk_buddy.h"
#include <errno.h>
int DEBUG = 0;

int main( 
    int   argc, 
    char *argv[] 
) {
    char    *file = "./store.sqlite";
    int      rc;
    int      i;
    int      pid;
    int      stat_loc;
    char    *event;
    char    *status;
    const char    *cp;
    const char    *command = "../do-sleep";
    struct upk_srvc s = {NULL, "package-1", "service-1" };

    printf("1..6\n");

    rc = upk_db_init( file, &s.pdb );

    if(rc < 0) {
	printf("db_init failed. Exiting.\n");
	exit(-1);
    }

    cp = upk_db_service_actual_status(  &s, UPK_STATUS_VALUE_STOP );
    upk_test_eq( cp, "stop", "service set to stop" );
    cp = upk_db_service_actual_status(  &s, UPK_STATUS_VALUE_START );
    upk_test_eq( cp, "start", "service set to start" );
    upk_db_service_cmdline(&s, command);
    cp = upk_db_service_desired_status(  &s, UPK_STATUS_VALUE_START );
    upk_test_eq( cp, "start", "desired status set to start" );
    pid = upk_buddy_start( &s, command, NULL, NULL);
    upk_test_isnt(pid,-1, "upk_buddy_start returned positive id");
    if( pid < 0 ) {
      printf( "upk_buddy_start failed (%d), %s\n", pid, strerror(errno) );
	exit( -1 );
    }

    upk_test_is (upk_buddy_stop( &s ),0, "stopping the buddy returned 0");
    upk_test_eq( cp, "stop");
    s.service="talker";
    upk_db_service_cmdline(&s, "../talker");
    pid = upk_buddy_start( &s, "../talker", NULL, NULL);
    upk_buddy_stop( &s );
    wait( &stat_loc );
    
    sqlite3_close( s.pdb );
    return 0;
}
