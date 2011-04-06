#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#include "upk_db.h"

extern int DEBUG;

static char EXEC_SINGLE_BUF[255];

static char *DB_PATH_MAIN = NULL;
static char *DB_PATH_MISC = NULL;

#define DB_PATH_MAIN_DEFAULT "../store/upkeeper-main.sqlite"
#define DB_PATH_MISC_DEFAULT "../store/upkeeper-misc.sqlite"

const char *upk_db_file_main( void ) {
    if( DB_PATH_MAIN == NULL ) {
        DB_PATH_MAIN = strdup( DB_PATH_MAIN_DEFAULT );
    }
    return DB_PATH_MAIN;
}

const char *upk_db_file_misc( void ) {
    if( DB_PATH_MISC == NULL ) {
        DB_PATH_MISC = strdup( DB_PATH_MISC_DEFAULT );
    }
    return DB_PATH_MISC;
}

void upk_db_file_main_set( const char *dbpath ) {
    if( DB_PATH_MAIN != NULL ) {
        free( DB_PATH_MAIN );
    }
    DB_PATH_MAIN = strdup( dbpath );
}

void upk_db_file_misc_set( const char *dbpath ) {
    if( DB_PATH_MISC != NULL ) {
        free( DB_PATH_MISC );
    }
    DB_PATH_MISC = strdup( dbpath );
}

char *upk_db_exec_single( 
    sqlite3  *pdb, 
    const char     *sql
) {
    int    rc;
    char  *zErr;
    int    nrow;
    int    ncol;
    char **result;

    if( DEBUG ) {
        printf("Running sql: %s\n", sql);
    }

    rc = sqlite3_get_table( pdb, sql, &result, 
	                    &nrow, &ncol, &zErr );

    if(rc != SQLITE_OK) {
	printf("Running '%s' failed: %s\n", sql, zErr);
	return(NULL);
    }

    if( nrow == 0 ) {
        /* Nothing found */
        return NULL;
    } else if ( nrow > 1 ) {
        printf("Running '%s' returned more than one result\n", sql);
        return(NULL);
    }

    if( result == NULL || result[1] == NULL ) {
        /* We didn't get a result */
        return(NULL);
    }

    if( strlen( result[1] ) > sizeof(EXEC_SINGLE_BUF) - 1 ) {
        sqlite3_free_table( result );
        printf("Result of '%s' too long.\n", sql);
        return(NULL);
    }

    strcpy( EXEC_SINGLE_BUF, result[ 1 ] );
    sqlite3_free_table( result );

    if( DEBUG ) {
        printf("upk_db_exec_single: return=%s\n", EXEC_SINGLE_BUF);
    }

    return(EXEC_SINGLE_BUF);
}

/* 
 * Format database date strings.
 */
char *
upk_db_time_now_mstring(
  void
) {
    time_t now;
    struct tm *now_tm;
    char      *date_string = sqlite3_mprintf("xxxx-xx-xx xx:xx:xxn");

    now = time(NULL);
    now_tm = localtime( &now );

    if (now_tm == NULL) {
        perror("localtime failed");
	sqlite3_free( date_string );
	return( NULL );
    }

    strftime(date_string, strlen((const char *) date_string),
	     "%Y-%m-%d %H:%M:%S", now_tm);

    return( date_string );
}

