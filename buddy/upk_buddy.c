#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#include "../store/upk_db.h"
#include "upk_buddy.h"

extern int DEBUG;

/* 
 * Launches the application process in argv[] with a 'buddy'
 * which updates the upkeeper database when the application has been started
 * and when it got stopped or crashed.
 */
int upk_buddy_start(
  sqlite3 *pdb, 
  char    *package,
  char    *service,
  char    *argv[], 
  char    *env[]
) {
}

/* 
 * Stops a previously launched buddied application process and updates
 * the upkeeper runtable information.
 */
int
upk_buddy_stop(
  sqlite3  *pdb, 
  int       buddy_pid
) {
    int    rc;
    char  *zErr;
    char  *sql;
    char  *id;
    int   found_id;
    char  *cp;

    sql = sqlite3_mprintf( "SELECT id from services" );
    sqlite3_free( sql );
}
