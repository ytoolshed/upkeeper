#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "../store/upk_db.h"

int DEBUG           = 1;
int OPT_ALL_UP,
    OPT_ALL_DOWN,
    OPT_HELP,
    OPT_INIT,
    OPT_DEFINE,
    OPT_UNDEFINE,
    OPT_SET_DESIRED_STATE,
    OPT_SET_ACTUAL_STATE,
    OPT_SET_PID,
    OPT_NONE
    = 0;

int main( 
    int   argc, 
    char *argv[] 
) {
    sqlite3 *pdb;
    char    *file = "../store/store.sqlite";
    int      rc;
    int      i;
    char    *service;
    char    *package;
    char   **cmdline;
    char     cmdline_buf[1024];
    int      nof_options;
    int      pid;

    nof_options = options_parse( argc, argv );

    if( nof_options == 0 || OPT_HELP ) {
	help();
	exit( 0 );
    }

    rc = upk_db_init( file, &pdb );

    if(rc < 0) {
	printf("upk_db_init failed. Exiting.\n");
	exit(-1);
    }
 
    if( OPT_INIT ) {

        upk_db_clear( pdb );

        for( i=0; i<=5; i++ ) {
            service = sqlite3_mprintf("service-%d", i);
            package = sqlite3_mprintf("package-%d", i);
            upk_db_service_actual_status( pdb, package, service, "stop");
            upk_db_service_desired_status( pdb, package, service, "start");
            sqlite3_free( service );
            sqlite3_free( package );
        }
    }

    if( OPT_ALL_UP || OPT_ALL_DOWN ) {

	char *status = "start";
	if( OPT_ALL_DOWN ) 
	    status = "stop";

        for( i=0; i<=5; i++ ) {
            service = sqlite3_mprintf("service-%d", i);
            package = sqlite3_mprintf("package-%d", i);

            upk_db_service_actual_status( pdb, package, service, status);

            sqlite3_free( service );
            sqlite3_free( package );
        }
    }

    if( OPT_DEFINE ) {
        if( argc < 5 ) {
            printf("usage: %s --define pkg srvc cmdline\n", argv[0]);
            exit(1);
        }
        package = argv[2];
        service = argv[3];
        cmdline = &argv[4];
        cmdline_join( argc - 4, cmdline, cmdline_buf, sizeof( cmdline_buf ) );

        if( DEBUG ) {
            printf( "Adding cmdline: '%s'\n", cmdline_buf );
        }
        upk_db_service_cmdline( pdb, package, service, cmdline_buf );
    }

    if( OPT_SET_PID ) {
        if( argc < 5 ) {
            printf("usage: %s --define pkg srvc cmdline\n", argv[0]);
            exit(1);
        }
        package = argv[2];
        service = argv[3];
        int pid = atoi( argv[4] );
	if( pid == 0 ) {
	    pid = -1;
	}
        upk_db_service_pid( pdb, package, service, pid );
    }

    if( OPT_SET_ACTUAL_STATE || OPT_SET_DESIRED_STATE ) {
        if( argc < 5 ) {
            printf("usage: %s --define pkg srvc state\n", argv[0]);
            exit(1);
        }
        package = argv[2];
        service = argv[3];
	if( OPT_SET_ACTUAL_STATE ) {
            upk_db_service_actual_status( pdb, package, service, argv[4] );
	} else {
            upk_db_service_desired_status( pdb, package, service, argv[4] );
	}
    }

    upk_db_listener_send_all_signals( pdb );

    sqlite3_close( pdb );

    return(0);
}

/* 
 * Command line join
 */
int cmdline_join( 
    int   argc,
    char *argv[], 
    char *buf,
    int   buf_len
) {
    int   i;
    char *cp = buf;

    if( DEBUG ) {
        printf( "Joining %d args\n", argc );
    }

    buf[0] = '\0';

    for( i=0; i<argc; i++ ) {

        if( DEBUG ) {
            printf( "Adding arg '%s' to buf\n", argv[i] );
        }

        if( cp - buf + strlen( argv[i] ) > buf_len - 2 ) {
            printf("Buffer overflow in cmdline_join (%ld > %d)\n",
                    cp - buf + strlen( argv[i] ), buf_len - 2 );
            return( -1 );
        }
        if( cp != buf ) {
            strcat( buf, " " );
        }
        strcat( buf, argv[i] );

        cp += strlen(buf) + 1;
    }

    if( DEBUG ) {
        printf( "Joined: '%s'\n", buf );
    }

    return( 0 );
}

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
        { "all-up",      0, &OPT_ALL_UP,   1 },
        { "all-down",    0, &OPT_ALL_DOWN, 1 },
        { "help",        0, &OPT_HELP, 1 },
        { "init",        0, &OPT_INIT, 1 },
        { "define",      0, &OPT_DEFINE, 1 },
        { "undefine",    0, &OPT_UNDEFINE, 1 },
        { "set-desired-state", 
                         0, &OPT_SET_DESIRED_STATE, 1 },
        { "set-actual-state", 
                         0, &OPT_SET_ACTUAL_STATE, 1 },
        { "set-pid",     0, &OPT_SET_PID, 1 },
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

    return nof_options;
}

/*
 * Print Help
 */
int help( ) {
    printf("usage: upk [options]\n\n");

    printf(" --help:     Print this help page\n");
    printf(" --all-up:   Set all services to 'start'\n");
    printf(" --all-down: Set all services to 'stop'\n");
    printf(" --define pkg srvc cmdline: Define a service\n");
    printf(" --undefine pkg srvc: Delete a service\n");
    printf(" --set-desired-state pkg srvc state: Set desired state\n");
    printf(" --set-actual-state pkg srvc state: Set actual state\n");
    printf(" --set-pid pkg srvc pid Set pid of a service process\n");
}
