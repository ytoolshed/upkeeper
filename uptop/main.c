#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "../store/upk_db.h"
#include <unistd.h>
#include "uptop.h"
#include <signal.h>
#include <ncurses.h>
#include <time.h>

#define COMPONENT "uptop"

int        DEBUG = 0;
sqlite3   *PDB;
static int OPT_VERSION = 0;

void uptop_signal_handler( int sig );
const char *time_as_string();
void clock_update ();
void cursor_corner ();

int main(
    int   argc, 
    char *argv[] 
) {
    char  *file = "../store/store.sqlite";
    int    rc;
    int    ch;
    struct timeval timeout = { 0, 100 };
    /*
      const struct timespec request = { 0, 1000000 };
      struct timespec remain;
    */

    options_parse( argc, argv );

    initscr(); /* Curses init */

    rc = upk_db_init( file, &PDB );

    if(rc < 0) {
	printf("db_init failed. Exiting.\n");
	exit(-1);
    }

    /* Register with the upkeeper listener service */

      /* remove previous leftovers */
    upk_db_listener_remove_dead( PDB ); 
    upk_db_listener_add( PDB, COMPONENT, getpid(), SIGUSR1 );

    (void) signal( SIGUSR1, uptop_signal_handler );

    uptop_services_print( PDB );

    nodelay( stdscr, TRUE );

    while(1) {
	/* use select for sleeping 100ms */
        sleep(1);
        /* select( 0, NULL, NULL, NULL, &timeout ); */
        /* clock_nanosleep( 0, 0, &request, &remain ); */

	ch = getch();
	if( ch != ERR ) {
	    break;
	}

        clock_update();
	cursor_corner();
    }

    upk_db_close( PDB );

    endwin(); /* Curses exit */

    return(0);
}

/* 
 * Handle refresh signal
 */
void uptop_signal_handler(
    int sig
) {
    switch( sig ) {
        case SIGUSR1:
            initscr();
            refresh();
            uptop_services_print( PDB );
            break;
        case SIGTERM:
            endwin(); /* Curses exit */
            upk_db_close( PDB );
            exit( 0 );
            break;
    }
}

/* 
 * Status iterator callback to reset all states to 'stop'.
 */
void uptop_print_callback( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service,
    char    *status_desired,
    char    *status_actual
) {

    char       *fq_service;
    const char *cmdline;
    int         pid;

    if( status_actual == NULL ) {
	status_actual = "UNDEF";
    }

    fq_service = sqlite3_mprintf(
	    "%s-%s", package, service);

    printw( "%-20s: ", fq_service );
    if( strcmp( status_actual, status_desired ) != 0 ) {
	attron( A_BOLD );
    }
    printw( "%-5s", status_actual );
    attroff( A_BOLD );
    printw( " [%-5s]", status_desired );

    pid = upk_db_service_pid( pdb, package, service, 0 );
    if( pid == 0 ) {
        printw( " [     ]" );
    } else {
        printw( " [%5d]", pid );
    }

    cmdline = upk_db_service_cmdline( pdb, package, service, NULL );
    if( cmdline == NULL ) {
        cmdline = "";
    }
    printw( " %s", cmdline );

    printw( "\n" );

    sqlite3_free( fq_service );
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
    static struct option long_options[] = {
        { "version", 0, &OPT_VERSION, 1 },
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
    }
}

/* 
 * Print the status of all services
 */
void uptop_services_print(
    sqlite3 *pdb
) {
    int x,y;

    clear();
    attron( A_BOLD );
    printw( "upkeeper 1.0 dashboard\n\n" );
    attroff( A_BOLD );
    printw( "USR1 me at pid %d\n", getpid() );
    printw( "----------------------------------------\n" );
    upk_db_status_checker( pdb, uptop_print_callback );
    printw( "----------------------------------------\n" );

    getmaxyx( stdscr, y, x );

    move(y-2, 0);
    printw( "Updated at: %s\n", time_as_string() );

    refresh(); /* Update Curses */
}

/* 
 * Update the ticking clock
 */
void clock_update (
) {
    int x,y;

    getmaxyx( stdscr, y, x );

    move(y-1, 0);
    printw( "Time now  : %s\n", time_as_string() );
}

/* 
 * Put the cursor in the corner
 */
void cursor_corner (
) {
    int x,y;

    getmaxyx( stdscr, y, x );

    move(y-1, x-1);
}

/* 
 * Get current time as string
 */
const char *time_as_string (
) {
    const time_t now     = time( NULL );
    const char *time_str = ctime( &now );

    return time_str;
}
