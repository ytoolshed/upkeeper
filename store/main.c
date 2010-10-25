#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "upk_db.h"

int DEBUG = 0;

int main( 
    int   argc, 
    char *argv[] 
) {
    char    *file = "store.sqlite";
    int      rc;
    int      i;
    const char    *cp;
    struct upk_srvc s = {NULL, "package", "service-1" };

    printf("1..10\n");

    /* test */
    upk_test_is( 1, 1, "one is one" );

    rc = upk_db_init( file, &s.pdb );
    
        /* setter */
    upk_db_service_cmdline( &s, "cmdline" );
        /* getter */
    upk_test_eq( upk_db_service_cmdline( &s, NULL ),
                 "cmdline" );

        /* setter */
    upk_db_service_pid( &s, 123 );
        /* getter */
    upk_test_is( upk_db_service_pid( &s, 0 ),
                 123, "get 123 back as pid" );

    if(rc < 0) {
	printf("upk_db_init failed. Exiting.\n");
	exit(-1);
    }
 
    for( i=0; i<=2; i++ ) {
      s.service = sqlite3_mprintf("service-%d", i);

      rc = upk_db_service_find_or_create( &s );
    }

    for( i=0; i<=5; i++ ) {
      
      s.service = sqlite3_mprintf("service-%d", i);
      s.package = sqlite3_mprintf("package-%d", i);
      cp = upk_db_service_actual_status( &s ,  UPK_STATUS_VALUE_STOP);
      upk_test_eq( cp, "stop" );
      cp = upk_db_service_desired_status( &s , UPK_STATUS_VALUE_START);
      upk_test_eq( cp, "start" );
      sqlite3_free( s.service );
      sqlite3_free( s.package );
    }
    
    s.service = "service-10";
    s.package = "package-10";
    upk_db_service_actual_status( &s , UPK_STATUS_VALUE_STOP);

    s.service = "service-1"; s.package = "package-1";
    cp = upk_db_service_desired_status( &s, 0 );    upk_test_eq( cp, "start" );
    s.service = "service-2"; s.package = "package-2";
    cp = upk_db_service_actual_status( &s, 0 );    upk_test_eq( cp, "stop" );
    s.service = "service-5"; s.package = "package-5";
    cp = upk_db_service_actual_status( &s, 0 );    upk_test_eq( cp, "stop" );
    s.service = "service-2"; s.package = "package-1";
    cp = upk_db_service_actual_status( &s, 0 );    upk_test_eq( cp, NULL );
    s.service = "service-2"; s.package = "package-0";
    cp = upk_db_service_actual_status( &s, 0 );    upk_test_eq( cp, NULL );
    s.service = "service-9"; s.package = "package-1";
    cp = upk_db_service_actual_status( &s, 0 );    upk_test_eq( cp, NULL );
    s.service = "service-10"; s.package = "package-10";
    cp = upk_db_service_actual_status( &s, 0 );    upk_test_eq( cp, "stop" );
    s.service = "service-9"; s.package = "package-1";
    cp = upk_db_service_actual_status( &s, 0 );    upk_test_eq( cp, NULL);
    upk_db_status_checker( s.pdb, upk_db_status_checker_launchcallback );

    /* upk_db_exec_single( pdb, "SELECT signal_send( 456456, 1 )" ); */

    upk_db_listener_send_all_signals( s.pdb );
    sqlite3_close( s.pdb );

    return(0);
}
