#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

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
db_init(
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

    return(0);
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

void upk_db_status_checker( 
    sqlite3 *pdb, 
    void (*callback)()
) {
    /* */
    callback( pdb, "p", "s", "d", "a" );
}
