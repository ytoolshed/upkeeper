#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "store/upk_db.h"
#include "upk_buddy.h"
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
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
    int      sock;
    char    *event;
    char    *status;
    const char    *cp;
    const char    *command = "../do-sleep";
    struct upk_srvc s = {NULL, "package-1", "service-1" };
    char msg[9];
    printf("1..17\n");

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
    pid = upk_buddy_start( &s, command, NULL);
    upk_test_isnt(pid,-1, "upk_buddy_start returned positive id");
    if( pid < 0 ) {
      printf( "upk_buddy_start failed (%d), %s\n", pid, strerror(errno) );
	exit( -1 );
    }
    while ((sock = upk_buddy_connect(pid)) == -1) {
      sleep(1);
    }
    write(sock,"s",1);
    read(sock,msg,13);
    upk_test_is(msg[0],'u',"got an up message");
    upk_test_is(*((int *)(msg+1)),pid,"with forked pid");
    upk_test_is (upk_buddy_stop( &s ),0, "stopping the buddy returned 0");
    upk_test_is(read(sock,msg,1+sizeof(int)*3),1+sizeof(int)*3,"read expected amount");
    upk_test_is(msg[0],'d',"got a down message");
    upk_test_is(read(sock,msg,1+sizeof(int)*3),1+sizeof(int)*3,"read expected amount");
    upk_test_is(msg[0],'e',"got an exit message");
    close(sock);
    s.service="talker";
    upk_db_service_cmdline(&s, "../talker");
    pid = upk_buddy_start( &s, "../talker", NULL);

    while ((sock = upk_buddy_connect(pid)) == -1) {
      sleep(1);
    }
    write(sock,"s",1);
    upk_test_is(read(sock,msg,1+sizeof(int)*3),1+sizeof(int)*3,"read expected amount");
    upk_test_is(msg[0],'u',"got an up message");
    upk_test_is(*((int *)(msg+1)),pid,"with forked pid");
    upk_buddy_stop( &s );
    upk_test_is(read(sock,msg,1+sizeof(int)*3),1+sizeof(int)*3,"read expected amount");
    upk_test_is(msg[0],'d',"got a down message");
    upk_test_is(read(sock,msg,1+sizeof(int)*3),1+sizeof(int)*3,"read expected amount");
    wait( &stat_loc );
    
    sqlite3_close( s.pdb );
    return 0;
}
