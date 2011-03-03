#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "upk_db.h"
#include "common/nonblock.h"

const char *upk_states[] = { "unknown", "start", "stop", "exited", "invalid" };

char       *upk_db_create_time = NULL;
int         upk_db_ino         = 0;

extern int DEBUG;


const char *_upk_db_service_status( 
    upk_srvc_t    srvc,                                   
    upk_state     state,
    int           type
);

static int db_init_functions_define( sqlite3 *pdb );

/* 
 * Initialize the DB connection, create the DB file if it doesn't 
 * exist yet.
 */
int 
upk_db_init(
    struct upk_db *pupk_db
) {
    int rc;

    if( (rc = upk_db_open( upk_db_file_main(), &pupk_db->pdb )) != 0 ) {
        return( rc );
    }

    if( (rc = upk_db_open( upk_db_file_misc(), &pupk_db->pdb_misc )) != 0 ) {
        return( rc );
    }

    rc = db_init_functions_define( pupk_db->pdb );

    if(rc != 0) {
	printf("Defining db extensions failed: %d\n", rc );
	return(rc);
    }

    upk_db_changed( pupk_db->pdb ); /* Start watching DB */

    return(0);
}

/* 
 * Open sqlite DB connection
 */
int 
upk_db_open(
    const char     *file, 
    sqlite3       **ppdb
) {
    int      rc;
    
      /* Check if database has been set up at all */
    rc = open( file, O_RDONLY );

    if(rc < 0) {
	printf("Can't read DB file '%s'.\n", file);
	return(rc);
    }
    
    close(rc);

    rc = sqlite3_open( file, ppdb );

    if(rc != 0) {
	printf("sqlite3_open '%s' failed: %d\n", file, rc);
	return(rc);
    }

    sqlite3_busy_timeout( *ppdb, 20000 );

    return( 0 );
}

/* Read out when the DB was created (i.e. the schema was set up) */
char *upk_db_created( 
    sqlite3 *pdb
) {
    char *sql;
    char *result;
    
    sql = sqlite3_mprintf( 
	    "SELECT value from namevalue WHERE name = 'created' ");

    result = upk_db_exec_single( pdb, sql );

    sqlite3_free( sql );

    return( result );
}

/* Check if DB has changed under our watch */
int upk_db_changed( 
    sqlite3 *pdb
) {
    char        *db_create_time;
    struct stat  statbuf;

    assert( stat( upk_db_file_main(), &statbuf ) == 0 );

    if( upk_db_ino) {
	assert( upk_db_ino == statbuf.st_ino );
    }

    upk_db_ino = statbuf.st_ino;

    db_create_time = upk_db_created( pdb );

    assert( db_create_time != NULL );

    if( ! upk_db_create_time ||
	  strcmp( db_create_time, upk_db_create_time ) == 0 ) {
	if( upk_db_create_time != NULL ) {
	    free( upk_db_create_time );
	}
	upk_db_create_time = strdup( db_create_time );
	return( 0 );
    }

    return( 1 );
}

/* 
 * Close the DB connection.
 */
int 
upk_db_exit(
    struct upk_db *pupk_db
) {
    int rc;

    if( (rc = sqlite3_close( pupk_db->pdb )) != 0 ) {
        return( rc );
    }

    if( (rc = sqlite3_close( pupk_db->pdb_misc )) != 0 ) {
        return( rc );
    }

    return( 0 );
}

/* Send a Unix signal to a process. Usage:
 *  SELECT signal_send( pid, signal_number );
 *  XXX: ERROR HANDLING
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

    pid       = sqlite3_value_int(values[0]);
    signal_no = sqlite3_value_int(values[1]);

    if( DEBUG ) {
        printf( "Sending signal %d to process %d\n", signal_no, pid );
        
    }
    rc = kill( pid, signal_no );

    if( rc != 0 ) {

	if( errno == ESRCH ) {
            /* Process already gone, ignore error. */
	} else {

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
    }

    sqlite3_result_int( ctx, 0 );
}

void get_pid(
    sqlite3_context *ctx, 
    int              nargs, 
    sqlite3_value  **values
) {
  sqlite3_result_int( ctx, getpid() );
}

static int db_init_functions_define( sqlite3 *pdb ) {
    int      rc;

    /* SELECT signal_send( signal_number, pid )
     */
    rc = sqlite3_create_function( pdb, "signal_send", 
	                     2,  /* args */
			     SQLITE_UTF8, NULL,
			     signal_send, NULL, NULL );
    rc = sqlite3_create_function( pdb, "get_pid", 
                                  0,
			     SQLITE_UTF8, NULL,
			     get_pid, NULL, NULL );
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
    const char    *package, 
    const char    *service,
    int      create
) {

    int    rc;
    char  *zErr;
    char  *sql;
    const char  *id;

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
   upk_srvc_t srvc
) {
    return( _upk_db_service_find( srvc->upk_db.pdb, 
                                  srvc->package, srvc->service, 0 ) );
}

/* 
 * Find an existing package/service combination and return its id (>=1)
 * Create it if it doesn't exist yet.
 */
int upk_db_service_find_or_create( 
   upk_srvc_t srvc
) {
    return( _upk_db_service_find( srvc->upk_db.pdb, 
                                  srvc->package, srvc->service, 1 ) );
}

/* 
 * Add a service event (start/stop/crashed/...).
 */
int 
_upk_db_event_add(
                  upk_srvc_t srvc,
                  const char    *event,
                  const char    *extra

) {

    int    rc;
    char  *zErr;
    char  *date_string = upk_db_time_now_mstring();
    int    service_id;
    char  *sql;

    service_id = upk_db_service_find_or_create( srvc );

    if( service_id < 0 ) {
        printf( "Can't create service\n" );
        return( UPK_ERROR_INTERNAL );
    }

    if( DEBUG ) {
        printf("upk_db_event_add %s %s %d\n", date_string, event, service_id);
    }

    sql = sqlite3_mprintf(
	          "INSERT INTO events (etime, event, service_id) "
                  "values (%Q, %Q%Q, %d)",
                  date_string, event, extra, service_id);

    rc = sqlite3_exec( srvc->upk_db.pdb, sql, NULL, NULL, &zErr );
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
const char *
_upk_db_service_status( 
    upk_srvc_t srvc,
    upk_state state,
    int     type
) {
    int    service_id;
    char  *sql;
    char  *state_field = "state_actual";
    int    rc;
    char *zErr;
    const char *status = upk_states[state];

    if( type == UPK_STATUS_DESIRED ) {
        state_field = "state_desired";
    }

    if( state ) {
      service_id = upk_db_service_find_or_create( srvc );

        sql = sqlite3_mprintf(
                "UPDATE services SET %s=%Q WHERE id = %d;",
	        state_field, status, service_id);

        rc = sqlite3_exec( srvc->upk_db.pdb, sql, NULL, NULL, &zErr );
        if( rc != SQLITE_OK ) {
	    printf( "Updating service state failed: %s\n", zErr );
	    return( NULL );
        }
        sqlite3_free( sql );
        _upk_db_event_add( srvc, 
                           type == UPK_STATUS_DESIRED ? "want " : "got " , 
                           status );
    }

    service_id = upk_db_service_find( srvc );

    if( service_id <= 0 && !state) {
        /* Service not found, so there can't be a status either */
        return( NULL );
    }

    sql = sqlite3_mprintf(
      "select %s from services where id = %d;",
      state_field, service_id);

    status = upk_db_exec_single( srvc->upk_db.pdb, sql );
    sqlite3_free( sql );

    return( status );
}

/* 
 * Get/Set the command line of a service.
 * Creates the procruns entry if it doesn't exist yet.
 */
const char *upk_db_service_desired_status( 
    upk_srvc_t    srvc,
    upk_state     state
) {
    return( _upk_db_service_status( srvc, state, UPK_STATUS_DESIRED ) );
}


const char *upk_db_service_cmdline( 
                                   upk_srvc_t srvc,
                                   const char *cmdline
) {
  char       *result;
  char       *sql;
  int         service_id;
  const char *id;

  if( DEBUG ) {
      printf("upk_db_service_cmdline %s/%s %s\n",
              srvc->package, srvc->service, cmdline);
  }

  sql = sqlite3_mprintf( "SELECT cmdline from services, procruns "
          "WHERE package = %Q "
          "AND service = %Q "
          "AND services.procrun_id = procruns.id ",
                         srvc->package, srvc->service );

  result =  upk_db_exec_single( srvc->upk_db.pdb, sql );
  sqlite3_free( sql );

  if( cmdline == NULL ) {
      /* 'get' calls end here */
      return ( result );
  }

  /* 'set' call */
  if( result == NULL ) {
      /* Entry doesn't exist yet, create a new one */
    service_id = upk_db_service_find_or_create( srvc );
      sql =     sqlite3_mprintf(
                    "INSERT INTO procruns (cmdline) "
                    "values (%Q)",
                    cmdline );
      upk_db_exec_single( srvc->upk_db.pdb, sql );
      sqlite3_free( sql );
      id = upk_db_exec_single( srvc->upk_db.pdb, 
                               "SELECT last_insert_rowid();" );  

      sql = sqlite3_mprintf("UPDATE services SET procrun_id=%s "
                            "WHERE id = %d ", id, service_id );
      upk_db_exec_single( srvc->upk_db.pdb, sql );
      sqlite3_free( sql );
  } else {
      /* Update an existing entry */
      sql = sqlite3_mprintf( "SELECT procrun_id from services, procruns "
              "WHERE services.procrun_id = procruns.id "
              "AND package = %Q "
              "AND service = %Q",
                             srvc->package, srvc->service );
      id = upk_db_exec_single( srvc->upk_db.pdb, sql );
      sqlite3_free( sql );

      sql = sqlite3_mprintf("UPDATE procruns SET cmdline=%Q "
                            "WHERE id = %d ", cmdline, atoi( id ));
      upk_db_exec_single( srvc->upk_db.pdb, sql );
      sqlite3_free( sql );
  }

  return( cmdline );
}

/* 
 * Get/Set the pid of a service.
 * Requires the procruns entry to exist.
 */
static int upk_db_get_pid( 
                       upk_srvc_t srvc,
                       char       *fieldname
) {
  char       *result, *sql;
  int         pid;

  sql = sqlite3_mprintf( "SELECT %s from services, procruns "
          "WHERE package = %Q "
          "AND service = %Q "
          "AND services.procrun_id = procruns.id ",
          fieldname,                
          srvc->package, srvc->service );

  result =  upk_db_exec_single( srvc->upk_db.pdb, sql );
  sqlite3_free( sql );

  /* 'get' calls end here */
  if( result == NULL ) {
    pid = 0;
  } else {
    pid = atoi( result );
  }
  return ( pid );
}


static int upk_db_set_pid(
                       upk_srvc_t srvc,
                       char *fieldname,
                       int pid ) {
  char       *sql, *id;
  sqlite3    *pdb = srvc->upk_db.pdb;

  /* Update an existing entry only! */
  sql = sqlite3_mprintf( "SELECT procrun_id from services, procruns "
          "WHERE services.procrun_id = procruns.id "
          "AND package = %Q "
          "AND service = %Q",
          srvc->package, srvc->service );
  id = upk_db_exec_single( pdb, sql );
  sqlite3_free( sql );
 
  if( pid < 0 ) {
      sql = sqlite3_mprintf("UPDATE procruns SET %s=NULL "
                            "WHERE id = %d ", fieldname, atoi( id ));
  } else {
      sql = sqlite3_mprintf("UPDATE procruns SET %s=%d "
                            "WHERE id = %d ", fieldname, pid, atoi( id ));
  }

  upk_db_exec_single( pdb, sql );
  sqlite3_free( sql );

  return( pid );
}

int upk_db_service_pid(upk_srvc_t srvc, int pid) {
  if (pid) { 
    return upk_db_set_pid(srvc,"pid",pid);
  }  
  return upk_db_get_pid(srvc,"pid");
  
}

int upk_db_service_buddy_pid(upk_srvc_t srvc, int pid) {
  if (pid) { 
    return upk_db_set_pid(srvc,"bpid",pid);
  }  
  return upk_db_get_pid(srvc,"bpid");
  
}

int upk_db_buddy_down(sqlite3 *pdb, int bpid) {
  char *sql;
  int  rc;
  char *zErr;
  sql = sqlite3_mprintf("BEGIN; UPDATE procruns SET pid=NULL, bpid=NULL "
                        "WHERE bpid = %d ; COMMIT;",bpid);
  
  rc = sqlite3_exec( pdb, sql, NULL, NULL, &zErr );
  sqlite3_free( sql );
  if(rc != SQLITE_OK) {
    fprintf(stderr,"set pid for buddy failed: %s\n", zErr);
    return( UPK_ERROR_INTERNAL );
  }

  return UPK_OK;
}


static int upk_do_buddy_down_event(
    sqlite3       *pdb, 
    sqlite3_stmt **down_stmt, 
    int            bpid, 
    int            status
) {
    int rc,i;
    char *zErr;

    if (SQLITE_OK != sqlite3_bind_int(down_stmt[0], 1, bpid)) {
      printf("Failed to bind integer parameter 1: %s\n","hmm");
      return 1;
    }

    if (SQLITE_OK != sqlite3_bind_int(down_stmt[1], 1, bpid)) {
      printf("Failed to bind integer parameter 1: %s\n","hmm");
      return 1;
    }
    
    if (SQLITE_OK != sqlite3_bind_int(down_stmt[2], 1, bpid)) {
      printf("Failed to bind integer parameter 2: %s\n","hmm");
      return 1;
    }

    if (SQLITE_OK != sqlite3_bind_int(down_stmt[2], 2, status)) {
      printf("Failed to bind integer parameter 2: %s\n","hmm");
      return 1;
    }

    for (i = 0; i < 3; i++) {
      rc = sqlite3_step( down_stmt[i] );

      if ( rc != SQLITE_DONE ) {
        printf("updating pids (%d,%d) failed: %d\n",status,bpid,rc);
        sqlite3_exec( pdb, "ROLLBACK;", NULL, NULL, &zErr );
        return 1;
      }
      sqlite3_reset( down_stmt[i] );
      sqlite3_clear_bindings( down_stmt[i] );
    }
    return 0;
}

static int 
upk_do_buddy_up_event(
    sqlite3       *pdb, 
    sqlite3_stmt **up_stmt, 
    int            bpid, 
    int            pid
) {

    int rc,i;
    char *zErr;

    if (SQLITE_OK != sqlite3_bind_int(up_stmt[0], 1, bpid)) {
      printf("Failed to bind integer parameter 1: %s\n","hmm");
      return 1;
    }

    if (SQLITE_OK != sqlite3_bind_int(up_stmt[1], 1, bpid)) {
      printf("Failed to bind integer parameter 1: %s\n","hmm");
      return 1;
    }
    
    if (SQLITE_OK != sqlite3_bind_int(up_stmt[1], 2, pid)) {
      printf("Failed to bind integer parameter 2: %s\n","hmm");
      return 1;
    }

    rc = sqlite3_step( up_stmt[0] );

    if ( rc != SQLITE_DONE ) {
      printf("updating pids (%d,%d) failed: %d\n",pid,bpid,rc);
      sqlite3_exec( pdb, "ROLLBACK;", NULL, NULL, &zErr );
      return 1;
    }
    rc = sqlite3_step( up_stmt[1] );

    if ( rc != SQLITE_DONE ) {
      printf("updating pids (%d,%d) failed: %d\n",pid,bpid,rc);
      sqlite3_exec( pdb, "ROLLBACK;", NULL, NULL, &zErr );
      return 1;
    }

    for (i = 0; i < 2; i++) {
      rc = sqlite3_step( up_stmt[i] );

      if ( rc != SQLITE_DONE ) {
        printf("updating pids (%d,%d) failed: %d\n",pid,bpid,rc);
        sqlite3_exec( pdb, "ROLLBACK;", NULL, NULL, &zErr );
        return 1;
      }
      sqlite3_reset( up_stmt[i] );
      sqlite3_clear_bindings( up_stmt[i] );
    }

    return 0;
}

int upk_db_update_buddy_events(
    struct upk_db *upk_db,
    int     *bpid,
    int     *pid, 
    int     *status
) {

  char *up_sql[3] =
    { "UPDATE services SET state_actual='start' WHERE procrun_id in "
      "(SELECT id from procruns where bpid = ?1); ",
      "UPDATE procruns SET pid= ?2 WHERE bpid = ?1 ;"
    };
  char *down_sql[4] =
    { "UPDATE procruns SET pid=NULL WHERE bpid = ?1 ;",
      "UPDATE services SET state_actual='stop' WHERE procrun_id in "
      "(SELECT id from procruns where bpid = ?1 );",
      "INSERT INTO exits (status,procrun_id)"
      "SELECT ?002, id from procruns where bpid=?1;"
    };

  int  rc;
  char *zErr;
  const char *zCerr;
  sqlite3_stmt *up_stmt[3], *down_stmt[4];
  int i;
  for (i = 0; i < 2; i++) {
    if (SQLITE_OK != sqlite3_prepare( upk_db->pdb, 
                                      up_sql[i], strlen(up_sql[i]), 
                                      &up_stmt[i], &zCerr )) {
      printf("Preparing up statement %d failed: %s\n", i, zCerr);
      return(UPK_DB_ERROR);
    }
  }  
  for (i = 0; i < 3; i++) {
    if (SQLITE_OK != sqlite3_prepare( upk_db->pdb, 
                                      down_sql[i], strlen(down_sql[i]), 
                                      &down_stmt[i], &zCerr )) {
      printf("Preparing down statement %d failed: %s\n", i, zCerr);
      return(UPK_DB_ERROR);
    }
  }
  rc = sqlite3_exec( upk_db->pdb, "BEGIN;", NULL, NULL, &zErr );
  if(rc != SQLITE_OK) {
    printf("Starting transaction failed: %s\n", zErr);
    return(UPK_DB_ERROR);
  }
  do {
    if (*pid) { /* when there's a pid we're doing an up */ 
      upk_do_buddy_up_event(upk_db->pdb, up_stmt, *bpid, *pid);
    } else {
      upk_do_buddy_down_event(upk_db->pdb,down_stmt, *bpid, *status);
    }
    pid++; status++;
  } while(*(++bpid));

  for (i = 0; i < 2; i++) {
    sqlite3_finalize( up_stmt[i] );
  }
  for (i = 0; i < 3; i++) {
    sqlite3_finalize( down_stmt[i] );
  }
  rc = sqlite3_exec( upk_db->pdb, "COMMIT;", NULL, NULL, &zErr );
  
  if(rc != SQLITE_OK) {
    printf("Ending transaction failed: %s\n", zErr);
    return(UPK_DB_ERROR);
  }
  upk_db_listener_send_all_signals(upk_db->pdb_misc);
  return UPK_OK;
}

/*
 *
 */
int upk_db_note_exit(
    struct upk_db *upk_db,
    int      status, 
    int      bpid
) {
  int bpids[2] = {0};
  int pids[2]  = {0};
  int stati[2] = {0};
  stati[0] = status;
  bpids[0] = bpid;
  return upk_db_update_buddy_events(upk_db,bpids,pids,stati);
}

/* 
 * Get/Set the a actual status of a service.
 */
const char * upk_db_service_actual_status( 
     upk_srvc_t srvc,
     upk_state  state
) {
  return( _upk_db_service_status( srvc, state, UPK_STATUS_ACTUAL ) );
}

/* 
 * Test callback for the status visitor, which just prints out all 
 * entries it is called with.
 */
void _upk_db_status_visitor_testcallback( 
    void *ignored,                                          
    upk_srvc_t srvc,                                    
    const char    *status_desired,
    const char    *status_actual
) {
    printf("Callback: %s-%s %s-%s\n",
            srvc->package, srvc->service, status_desired, status_actual);
}

/* 
 * Launcher callback for the status visitor. 
 * Starts up processes with actual=stop and desired=start.
 * Shuts down processes with actual=start and desired=stop.
 */
void upk_db_status_visitor_launchcallback( 
    void *ignored,                                          
    upk_srvc_t srvc,                                    
    const char    *status_desired,
    const char    *status_actual
) {
    if( status_desired == NULL ||
        status_actual  == NULL ) {
          /* Service not initialized, do nothing */
        return;
    }
    if ( DEBUG) {
    printf("Checker: Status of %s-%s is %s, but needs to be %s\n",
           srvc->package, srvc->service, status_actual, status_desired);
    printf("Launching %s-%s\n", srvc->package, srvc->service);
    }
}

/* 
 * Status visitor iterates through all configured pkg/services in the DB and
 * calls the provided callback functions for every entry it finds.
 */

struct {
  struct upk_srvc s;
  char *a;
  char *b;
  int bpid;
} cb[MAX_SERVICES];

void upk_db_status_visitor( 
    sqlite3 *pdb, 
    void (*callback)(),
    void *context

) {
    const char         *sql;
    sqlite3_stmt *stmt;
    int           rc, ncols;
    int count = 0;
    
    sql = "SELECT package, service, state_desired, state_actual "
          "FROM services "
	  "ORDER by package, service "
	  ";";

    sqlite3_prepare( pdb, sql, strlen(sql), &stmt, NULL );

    ncols = sqlite3_column_count( stmt );
    rc = sqlite3_step( stmt );

    while( rc == SQLITE_ROW ) {
      const char *     res = "";
      if ((res = sqlite3_column_text( stmt, 0 )) != NULL) res = strdup(res);
      cb[count].s.package = (char *)res;
      if ((res = sqlite3_column_text( stmt, 1 )) != NULL) res = strdup(res);
      cb[count].s.service = (char *)res;
      if ((res = sqlite3_column_text( stmt, 2 )) != NULL) res = strdup(res);
      cb[count].a         = (char *)res;
      if ((res = sqlite3_column_text( stmt, 3 )) != NULL) res = strdup(res);
      cb[count].b         = (char *)res;
      if ((res = sqlite3_column_text( stmt, 4 )) != NULL) {
        cb[count].bpid      = atoi(res);
      }
      cb[count].s.upk_db.pdb = pdb;
      count++;
      rc = sqlite3_step( stmt );
    }

    sqlite3_finalize( stmt );

    while (count--) {
      (*callback)( context, 
                   &cb[count].s,
                   cb[count].a,
                   cb[count].b, 
                   cb[count].bpid);

      if (cb[count].a)       free(cb[count].a);  
      if (cb[count].b)       free(cb[count].b);  
      if (cb[count].s.package) free(cb[count].s.package);  
      if (cb[count].s.service) free(cb[count].s.service);  
    }
}

/* 
 * Add a listener for a component. Nuke all existing ones for this component.
 */
int 
upk_db_listener_add(
    sqlite3    *pdb, 
    const char *component, 
    int         pid,
    int         signal
) {
    int   rc;
    char *zErr;
    char *sql = sqlite3_mprintf(
	          "INSERT INTO listeners (component, pid, signal) "
                  "values (%Q, %d, %d)",
	        component, pid, signal );

    rc = sqlite3_exec( pdb, sql, NULL, NULL, &zErr );

    if(rc != SQLITE_OK) {
	printf("Listener insert failed: %s\n", zErr);
	return( UPK_ERROR_INTERNAL );
    }

    sqlite3_free( sql );

    return( 0 );
}

/* 
 * Remove all listeners for a component.
 */
int 
upk_db_listener_remove(
    sqlite3    *pdb, 
    const char *component
) {
    int   rc;
    char *zErr;
    char *sql = sqlite3_mprintf(
	          "DELETE FROM listeners WHERE component = %Q",
	        component );

    rc = sqlite3_exec( pdb, sql, NULL, NULL, &zErr );

    if(rc != SQLITE_OK) {
	printf("Listener remove failed: %s\n", zErr);
	return( UPK_ERROR_INTERNAL );
    }

    sqlite3_free( sql );

    return( 0 );
}

/* 
 * Visitor callback to remove a dead listener.
 */
void 
_upk_db_dead_listener_remove_callback(
    sqlite3    *pdb,
    const char *component,
    int         pid,
    int         signal
) {
    int   rc;
    char *zErr;
    char *sql;
   
    if( kill( pid, 0 ) == 0 ) {
	/* Process is alive, ignore */
	return;
    }

    sql = sqlite3_mprintf(
	          "DELETE FROM listeners WHERE pid = %d",
	        pid );

    rc = sqlite3_exec( pdb, sql, NULL, NULL, &zErr );

    if(rc != SQLITE_OK) {
	printf("Listener remove failed: %s\n", zErr);
	return;
    }

    sqlite3_free( sql );

    return;
}

/* 
 * Visit all listeners for a component and call a callback with the pid
 * each time.
 */
void upk_db_listener_visitor( 
    sqlite3 *pdb, 
    void (*callback)()
) {
    const char   *sql;
    sqlite3_stmt *stmt;
    int           rc, ncols;

    sql = "SELECT component, pid, signal "
          "FROM listeners "
	  ";";

    sqlite3_prepare( pdb, sql, strlen(sql), &stmt, NULL );

    ncols = sqlite3_column_count( stmt );
    rc = sqlite3_step( stmt );

    while( rc == SQLITE_ROW ) {
        (*callback)( pdb,
                sqlite3_column_text( stmt, 0 ),
                sqlite3_column_int( stmt, 1 ),
                sqlite3_column_int( stmt, 2 )
                );
        rc = sqlite3_step( stmt );
    }

    sqlite3_finalize( stmt );
}

/* 
 * Callback to send signals to listeners
 */
void _upk_db_listener_signal_callback( 
    sqlite3    *pdb, 
    const char *component, 
    int         pid,
    int         signal
) {
    if( DEBUG ) {
         printf( "Sending signal to component %s: kill %d %d\n",
            component, pid, signal );
    }
    
    /* best effort only, don't care if that process actually exists */
    kill( pid, signal );
}

/* 
 * Deliver all signals
 */
void upk_db_listener_send_all_signals(
    sqlite3    *pdb
) {
    upk_db_listener_visitor( pdb, _upk_db_listener_signal_callback );
}

/* 
 * Remove dead listeners
 */
void upk_db_listener_remove_dead(
    sqlite3    *pdb
) {
    upk_db_listener_visitor( pdb, _upk_db_dead_listener_remove_callback );
}

/* 
 * Wipe all DB tables
 */
void upk_db_clear(
    struct upk_db *upk_db
) {
    upk_db_exec_single( upk_db->pdb, "DELETE FROM procruns;" );
    upk_db_exec_single( upk_db->pdb, "DELETE FROM services;" );
    upk_db_exec_single( upk_db->pdb, "DELETE FROM events;" );

    upk_db_exec_single( upk_db->pdb_misc, "DELETE FROM listeners;" );
}
