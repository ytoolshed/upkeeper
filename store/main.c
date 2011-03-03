#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "upk_db.h"
#include "common/test.h"

int DEBUG = 0;

int main( 
    int   argc, 
    char *argv[] 
) {
    int             rc;
    int             i;
    const char     *cp;
    struct upk_srvc s;

    s.package = "package";
    s.service = "service-1";

    printf("1..35\n");

    /* test */
    upk_test_is( 1, 1, "one is one" );

    rc = upk_db_init( &s.upk_db );

    
    if(rc < 0) {
	printf("upk_db_init failed. Exiting.\n");
	exit(-1);
    }
 
        /* setter */
    upk_db_service_cmdline( &s, "cmdline" );
        /* getter */
    upk_test_eq( upk_db_service_cmdline( &s, NULL ),
                 "cmdline", "roundtrip of upk_db_service_cmdline" );

        /* setter */
    upk_db_service_pid( &s, 123 );
        /* getter */
    upk_test_is( upk_db_service_pid( &s, 0 ),
                 123, "get 123 back as pid" );

        /* setter */
    upk_db_service_buddy_pid( &s, 244 );
        /* getter */
    upk_test_is( upk_db_service_buddy_pid( &s, 0 ),
                 244, "get 244 back as pid" );

    cp = upk_db_service_actual_status( &s, 0 );    
    upk_test_eq( cp, "stop", "beginning state is \"stop\"" );

    {
      int pids[2] = { 123, 0 };
      int bpids[2] = { 244, 0 };
      int status[2] = { -1, 0 };
      upk_db_update_buddy_events(&s.upk_db, bpids,pids,status);
    }
    upk_test_is( upk_db_service_pid( &s, 0 ),
                 123, "get 123 back as pid" );

    cp = upk_db_service_actual_status( &s, 0 );    
    upk_test_eq( cp, "start", "actual status start after up event");

    {
      int pids[2] = { 0, 0 };
      int bpids[2] = { 244, 0 };
      int status[2] = { 0, 0 };
      upk_db_update_buddy_events(&s.upk_db, bpids, pids, status);

    }
    upk_test_is( upk_db_service_pid( &s, 0 ),
                 0, "get 0 back as pid after down event " );

    upk_test_is( upk_db_service_buddy_pid( &s, 0 ),
                 244, "get 244 back as buddy pid" );

    cp = upk_db_service_actual_status( &s, 0 );    
    upk_test_eq( cp, "stop", "actual status stop" );



    {
      int pids[2] = { 123, 0 };
      int bpids[2] = { 244, 0 };
      int status[2] = { -1, 0 };
      upk_db_update_buddy_events(&s.upk_db, bpids,pids,status);
    }
    upk_test_is( upk_db_service_pid( &s, 0 ),
                 123, "get 123 back as pid" );


    cp = upk_db_service_actual_status( &s, 0 );    
    upk_test_eq( cp, "start", "upk_db_note_exit seen in status" );

    upk_test_is(upk_db_note_exit(&s.upk_db, 123, 244),
                0,
                "exit for known buddy");

    cp = upk_db_service_actual_status( &s, 0 );    
    upk_test_eq( cp, "stop", "upk_db_note_exit seen in status" );

    for( i=0; i<=2; i++ ) {
      s.service = sqlite3_mprintf("service-%d", i);

      rc = upk_db_service_find_or_create( &s );
    }

    for( i=0; i<=5; i++ ) {
      
      s.service = sqlite3_mprintf("service-%d", i);
      s.package = sqlite3_mprintf("package-%d", i);
      cp = upk_db_service_actual_status( &s ,  UPK_STATE_STOP);
      upk_test_eq( cp, "stop", "setting stop returns stop" );
      cp = upk_db_service_desired_status( &s , UPK_STATE_START);
      upk_test_eq( cp, "start", "setting start returns start" );
      sqlite3_free( s.service );
      sqlite3_free( s.package );
    }
    
    s.service = "service-10";
    s.package = "package-10";
    upk_db_service_actual_status( &s , UPK_STATE_STOP);

    s.service = "service-1"; s.package = "package-1";
    cp = upk_db_service_desired_status( &s, 0 );    upk_test_eq( cp, "start", "setting start for desired roundtrip" );
    s.service = "service-2"; s.package = "package-2";
    cp = upk_db_service_actual_status( &s, 0 );    upk_test_eq( cp, "stop", "setting stop for actual roundtrip" );
    s.service = "service-5"; s.package = "package-5";
    cp = upk_db_service_actual_status( &s, 0 );    upk_test_eq( cp, "stop", "got stop from earlier" );
    s.service = "service-2"; s.package = "package-1";
    cp = upk_db_service_actual_status( &s, 0 );    upk_test_eq( cp, NULL, "package-1 service-2 still NULL" );
    s.service = "service-2"; s.package = "package-0";
    cp = upk_db_service_actual_status( &s, 0 );    upk_test_eq( cp, NULL, "package-0 service-2 still NULL" );
    s.service = "service-9"; s.package = "package-1";
    cp = upk_db_service_actual_status( &s, 0 );    upk_test_eq( cp, NULL, "p-1, service-9 still null" );
    s.service = "service-10"; s.package = "package-10";
    cp = upk_db_service_actual_status( &s, 0 );    upk_test_eq( cp, "stop", "10,10 is stop" );
    s.service = "service-9"; s.package = "package-1";
    cp = upk_db_service_actual_status( &s, 0 );    upk_test_eq( cp, NULL, "9,1 is null");
    upk_db_status_visitor( s.upk_db.pdb, 
            upk_db_status_visitor_launchcallback, NULL );

    /* upk_db_exec_single( pdb, "SELECT signal_send( 456456, 1 )" ); */

    upk_db_listener_send_all_signals( s.upk_db.pdb_misc );

    upk_test_is( upk_db_changed( s.upk_db.pdb ), 0, "db change check" );

    sqlite3_close( s.upk_db.pdb );

    return(0);
}
