#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include "upk_db.h"

extern int DEBUG;

int _upk_db_event_add( 
    sqlite3 *pdb, 
    char    *event, 
    char    *package, 
    char    *service
);

char *_upk_db_service_status( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service,
    char    *new_status,
    int     type
);

/* 
 * Initialize the DB connection, create the DB file if it doesn't 
 * exist yet.
 */
int 
upk_db_init(
    char     *file, 
    sqlite3 **ppdb 
) {
    int      rc;

      /* Check if database has been set up at all */
    rc = open( file, O_RDONLY );

    if(rc < 0) {
	printf("Can't read DB file '%s'.\n", file);
	return(rc);
    }

    rc = sqlite3_open( file, ppdb );

    if(rc != 0) {
	printf("sqlite3_open '%s' failed: %d\n", file, rc);
	return(rc);
    }

    rc = upk_db_init_functions_define( *ppdb );

    if(rc != 0) {
	printf("Defining db extensions failed: %d\n", rc );
	return(rc);
    }

    return(0);
}

/* 
 * Close the DB connection.
 */
int 
upk_db_close(
    sqlite3 *pdb 
) {
    sqlite3_close( pdb );
}

/* Send a Unix signal to a process. Usage:
 *  SELECT signal_send( pid, signal_number );
 */
void signal_send(
    sqlite3_context *ctx, 
    int              nargs, 
    sqlite3_value  **values
) {
    int   signal_no;
    int   pid;
    int   rc;
    char *message;

    pid       = atoi( sqlite3_value_text( values[0] ) );
    signal_no = atoi( sqlite3_value_text( values[1] ) );

    if( DEBUG ) {
        printf( "Sending signal %d to process %d\n", signal_no, pid );
    }

    rc = kill( (pid_t) pid, signal_no );

    if( rc != 0 ) {

        message = sqlite3_mprintf(
	    "kill(%d, %d) failed: %s", 
	    pid, signal_no, strerror( errno ) );

	if( DEBUG ) {
	    printf( "%s\n", message );
	}

	sqlite3_result_error( ctx, message, strlen( message ) ); 
	sqlite3_free( message );
	return;
    }

    sqlite3_result_int( ctx, 0 );
}

int upk_db_init_functions_define( sqlite3 *pdb ) {
    int      rc;

    /* SELECT signal_send( signal_number, pid )
     */
    rc = sqlite3_create_function( pdb, "signal_send", 
	                     2,  /* args */
			     SQLITE_UTF8, NULL,
			     signal_send, NULL, NULL );
    if( rc != 0 ) {
	return( rc );
    }

    return( 0 );
}

/* 
 * Find an existing package/service combination and return its id.
 */
int _upk_db_service_find( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service,
    int      create
) {

    int    rc;
    char  *zErr;
    char  *sql;
    char  *id;
    int   found_id;
    char  *cp;

    /* We're using a transaction here to avoid the race condition
     * which occurs when we first search for an existing record and
     * then create it if it doesn't exist.
     */

    rc = sqlite3_exec( pdb, "BEGIN;", NULL, NULL, &zErr );
    if(rc != SQLITE_OK) {
	printf("Starting transaction failed: %s\n", zErr);
	return(UPK_DB_ERROR);
    }

    sql = sqlite3_mprintf( "SELECT id from services WHERE package = %Q "
                           "AND service = %Q", package, service );
    id = upk_db_exec_single( pdb, sql );
    sqlite3_free( sql );

    if( create && id == NULL ) {
        /* Doesn't exist yet. Create a new one. */
        sql = sqlite3_mprintf( "INSERT INTO services "
            "(package, service) VALUES (%Q, %Q);",
            package, service );
        upk_db_exec_single( pdb, sql );
        id = upk_db_exec_single( pdb, "SELECT last_insert_rowid();" );
    }

    rc = sqlite3_exec( pdb, "COMMIT;", NULL, NULL, &zErr );
    if(rc != SQLITE_OK) {
	printf("Ending transaction failed: %s\n", zErr);
	return(rc);
    }

    if( id == NULL) {
        if( create ) {
	    printf("Failed to add service '%s/%s': %s\n", 
                   package, service, zErr);
            return(-1);
        } else {
            return( 0 );
        }
    }

    if( DEBUG ) {
        printf("ID found is '%s'\n", id);
    }

    return atoi( id );
}

/* 
 * Find an existing package/service combination and return its id (>=1)
 */
int upk_db_service_find( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service
) {
    return( _upk_db_service_find( pdb, package, service, 0 ) );
}

/* 
 * Find an existing package/service combination and return its id (>=1)
 * Create it if it doesn't exist yet.
 */
int upk_db_service_find_or_create( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service
) {
    return( _upk_db_service_find( pdb, package, service, 1 ) );
}

/* 
 * Add a service event (start/stop/crashed/...).
 */
int 
_upk_db_event_add(
    sqlite3 *pdb, 
    char    *event, 
    char    *package, 
    char    *service
) {

    int    rc;
    char  *zErr;
    char  *date_string = upk_db_time_now_mstring();
    time_t now;
    int    service_id;
    struct tm *now_tm;

    service_id = upk_db_service_find_or_create( pdb, package, service );

    if( service_id < 0 ) {
        printf( "Can't create service\n" );
        return( UPK_ERROR_INTERNAL );
    }

    if( DEBUG ) {
        printf("Inserting %s %s %d\n", date_string, event, service_id);
    }

    char *sql = sqlite3_mprintf(
	          "INSERT INTO events (etime, event, service_id) "
                  "values (%Q, %Q, %d)",
	        date_string, event, service_id);

    rc = sqlite3_exec( pdb, sql, NULL, NULL, &zErr );
    sqlite3_free( date_string );

    if(rc != SQLITE_OK) {
	printf("Event insert failed: %s\n", zErr);
	return( UPK_ERROR_INTERNAL );
    }

    sqlite3_free( sql );
    return(0);
}

/* 
 * Get/Set the a actual/desired status of a service.
 */
char *
_upk_db_service_status( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service,
    char    *new_status,
    int     type
) {
    int    service_id;
    char  *sql;
    char  *state_field = "state_actual";
    int    rc;
    char *zErr;
    char *status;

    if( type == UPK_STATUS_DESIRED ) {
        state_field = "state_desired";
    }

    if( new_status ) {
        service_id = upk_db_service_find_or_create( pdb, package, service );

        sql = sqlite3_mprintf(
                "UPDATE services SET %s=%Q WHERE id = %d;",
	        state_field, new_status, service_id);

        rc = sqlite3_exec( pdb, sql, NULL, NULL, &zErr );
        if( rc != SQLITE_OK ) {
	    printf( "Updating service state failed: %s\n", zErr );
	    return( NULL );
        }
        sqlite3_free( sql );

        _upk_db_event_add( pdb, new_status, package, service );
    }

    service_id = upk_db_service_find( pdb, package, service );

    if( service_id <= 0 && new_status == NULL ) {
        /* Service not found, so there can't be a status either */
        return( NULL );
    }

    sql = sqlite3_mprintf(
      "select %s from services where id = %d;",
      state_field, service_id);

    status = upk_db_exec_single( pdb, sql );
    sqlite3_free( sql );

    return( status );
}

/* 
 * Get/Set the a actual status of a service.
 */
char *upk_db_service_actual_status( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service,
    char    *new_status
) {
    return( _upk_db_service_status( pdb, package, service, new_status, 
                                    UPK_STATUS_ACTUAL ) );
}

/* 
 * Get/Set the a desired status of a service.
 */
char *upk_db_service_desired_status( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service,
    char    *new_status
) {
    return( _upk_db_service_status( pdb, package, service, new_status, 
                                    UPK_STATUS_DESIRED ) );
}

/* 
 * Test callback for the status checker, which just prints out all 
 * entries it is called with.
 */
void _upk_db_status_checker_testcallback( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service,
    char    *status_desired,
    char    *status_actual
) {
    printf("Callback: %s-%s %s-%s\n",
            package, service, status_desired, status_actual);
}

/* 
 * Launcher callback for the status checker. 
 * Starts up processes with actual=stop and desired=start.
 * Shuts down processes with actual=start and desired=stop.
 */
void upk_db_status_checker_launchcallback( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service,
    char    *status_desired,
    char    *status_actual
) {
    if( status_desired == NULL ||
        status_actual  == NULL ) {
          /* Service not initialized, do nothing */
        return;
    }

    if( !strcmp( status_actual, UPK_STATUS_VALUE_STOP ) &&
        !strcmp( status_desired, UPK_STATUS_VALUE_START ) ) {
        printf("Checker: Status of %s-%s is %s, but needs to be %s\n",
                package, service, status_actual, status_desired);
        printf("Launching %s-%s\n", package, service);
    }
}

/* 
 * Status checker iterates through all configured pkg/services in the DB and
 * calls the provided callback functions for every entry it finds.
 */
void upk_db_status_checker( 
    sqlite3 *pdb, 
    void (*callback)()
) {
    char         *sql;
    sqlite3_stmt *stmt;
    int           rc, ncols;

    sql = "SELECT package, service, state_desired, state_actual "
          "FROM services;";

    sqlite3_prepare( pdb, sql, strlen(sql), &stmt, NULL );

    ncols = sqlite3_column_count( stmt );
    rc = sqlite3_step( stmt );

    while( rc == SQLITE_ROW ) {
        (*callback)( pdb, 
                sqlite3_column_text( stmt, 0 ),
                sqlite3_column_text( stmt, 1 ),
                sqlite3_column_text( stmt, 2 ),
                sqlite3_column_text( stmt, 3 )
                );
        rc = sqlite3_step( stmt );
    }

    sqlite3_finalize( stmt );
}
