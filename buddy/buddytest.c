#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include "store/upk_db.h"
#include "common/test.h"
#include "upk_buddy.h"

int DEBUG = 0;

int main( 
    int   argc, 
    char *argv[] 
) {
    char    *file = upk_db_file_main();
    int      rc;
    int      pid;
    int      stat_loc;
    int      sock;
    const char    *cp;
    const char    *command = "../do-sleep";
    struct upk_srvc s;
    int limit = 5;
    char msg[128];
    printf("1..22\n");

    s.package = "package-1";
    s.service = "service-1";

    rc = upk_db_init( &s.upk_db );

    if(rc < 0) {
	printf("db_init failed. Exiting.\n");
	exit(-1);
    }

    cp = upk_db_service_actual_status(  &s, UPK_STATE_STOP );
    upk_test_eq( cp, "stop", "service set to stop" );
    cp = upk_db_service_actual_status(  &s, UPK_STATE_START );
    upk_test_eq( cp, "start", "service set to start" );
    upk_db_service_cmdline(&s, command);
    cp = upk_db_service_desired_status(  &s, UPK_STATE_START );
    upk_test_eq( cp, "start", "desired status set to start" );
    pid = upk_buddy_start( &s, command, NULL);
    upk_test_isnt(pid,-1, "upk_buddy_start returned positive id");
    if( pid < 0 ) {
      printf( "upk_buddy_start failed (%d), %s\n", pid, strerror(errno) );
      exit( -1 );
    }

    while ((sock = upk_buddy_connect(pid)) == -1) {
      limit--;
      sleep(1);
      if (limit==0) {
        upk_test_is(0,1,"can't find buddy for talker");
        break;
      }
    }

    upk_test_is(write(sock,"s",1),1,"Wrote status request");
    upk_test_is(read(sock,msg,1+sizeof(int)*3),1+sizeof(int)*3,"read expected amount");
    upk_test_is(msg[0],'u',"got an up message");
    upk_test_is(*((int *)(msg+1)),pid,"with forked pid");

    upk_test_is (upk_buddy_stop( pid ),0, "stopping the buddy returned 0");

    memset(msg,0,sizeof(msg));
    upk_test_is(read(sock,msg,1+sizeof(int)*3),1+sizeof(int)*3,"read expected amount");
    upk_test_is(msg[0],'d',"got a down message");


    memset(msg,0,sizeof(msg));
    upk_test_is(read(sock,msg,1+sizeof(int)*3),1+sizeof(int)*3,"read expected amount");
    upk_test_is(msg[0],'e',"got an exit message");

    close(sock);
    s.service="talker";
    upk_db_service_cmdline(&s, "../talker");
    pid = upk_buddy_start( &s, "../talker", NULL);

    upk_test_isnt(pid,-1, "upk_buddy_start returned positive id");

    limit = 5;
    while ((sock = upk_buddy_connect(pid)) == -1) {
      limit--;
      sleep(1);
      if (limit==0) {
        upk_test_is(0,1,"can't find buddy for talker");
        break;
      }
    }

    upk_test_is(write(sock,"s",1),1,"wrote 1 for status");

    memset(msg,0,sizeof(msg));
    upk_test_is(read(sock,msg,1+sizeof(int)*3),1+sizeof(int)*3,"read expected amount");
    upk_test_is(*((int *)(msg+1)),pid,"from forked pid");
    upk_test_is(upk_buddy_stop( pid ), 0, "stopping buddy returned 0");

    memset(msg,0,sizeof(msg));
    upk_test_is(read(sock,msg,1+sizeof(int)*3),1+sizeof(int)*3,"read expected amount");
    upk_test_is(*((int *)(msg+1)),pid,"from forked pid");

    memset(msg,0,sizeof(msg));
    upk_test_is(read(sock,msg,1+sizeof(int)*3),1+sizeof(int)*3,"read expected amount");
    upk_test_is(*((int *)(msg+1)),pid,"from forked pid");
 
    wait( &stat_loc );
    
    upk_db_exit( &s.upk_db );
    return 0;
}
