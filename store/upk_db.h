#ifndef UPK_DB_H
#define UPK_DB_H


/* Upkeeper constants */
#define MAX_SERVICES 1024



#define UPK_OK                     0
#define UPK_ERROR_SERVICE_UNKNOWN -1
#define UPK_ERROR_PACKAGE_UNKNOWN -2
#define UPK_ERROR_INTERNAL        -3
#define UPK_DB_ERROR              -4
#define UPK_ERROR_BUDDY_UNKNOWN   -5

/* Prototypes */

#include <sqlite3.h>

typedef struct upk_db {
    sqlite3 *pdb;
    sqlite3 *pdb_misc;
} upk_db_t;

int upk_db_init( struct upk_db *pupk_db );

int upk_db_exit(
    struct upk_db *pupk_db
);

typedef enum { 
  UPK_STATE_UNKNOWN = 0,    
  UPK_STATE_START   = 1,
  UPK_STATE_STOP    = 2,
  UPK_STATE_EXITED  = 3,
  UPK_STATE_INVALID = 4,
  UPK_STATE_MAX     = 5
} upk_state;

typedef enum {
  UPK_STATUS_ACTUAL  = 0,
  UPK_STATUS_DESIRED
} upk_status_t;

extern const char *upk_states[];

typedef struct upk_srvc {
  struct upk_db upk_db;
  char         *package;
  char         *service;
} *upk_srvc_t;


const char *
upk_db_service_actual_status( 
    upk_srvc_t srvc,
    upk_state  state
);

/* getter/setter. */
const char *
upk_db_service_desired_status( 
    upk_srvc_t srvc,
    upk_state  state
);

/* hmm */
char *upk_db_time_now_mstring( void );

/* 
 * Speak to memory allocation ? */
char *upk_db_exec_single( 
    sqlite3  *pdb, 
    const char     *sql
);



int upk_db_service_find_or_create( upk_srvc_t );
int upk_db_service_find( upk_srvc_t );

void upk_db_status_visitor( 
    sqlite3 *pdb, 
    void (*callback)(),
    void  *context
);

void _upk_db_status_visitor_testcallback( 
    void *context,                                         
    upk_srvc_t    srvc,                                         
    const char    *status_desired,
    const char    *status_actual
);

void upk_db_status_visitor_launchcallback( 
    void *context,                                         
    upk_srvc_t    srvc,                                         
    const char    *status_desired,
    const char    *status_actual
);

int 
upk_db_listener_add(
    sqlite3    *pdb, 
    const char *component, 
    int         pid,
    int         signal
);

int 
upk_db_listener_remove(
    sqlite3    *pdb, 
    const char *component
);

void upk_db_listener_visitor( 
    sqlite3 *pdb, 
    void (*callback)()
);

void upk_db_listener_send_all_signals( sqlite3 *pdb );

void 
_upk_db_dead_listener_remove_callback(
    sqlite3    *pdb,
    const char *component,
    int         pid,
    int         signal
);

void upk_db_listener_remove_dead(
    sqlite3    *pdb
);

void upk_db_clear(
    struct upk_db *upk_db
);

const char *upk_db_service_cmdline( 
                                   upk_srvc_t svc,
                                   const char *cmdline
);

int upk_db_service_pid( 
                                   upk_srvc_t svc,
                                   int         pid
);
int upk_db_service_buddy_pid( 
                                   upk_srvc_t svc,
                                   int         pid
);
int upk_db_buddy_down(sqlite3 *pdb, int bpid);

int upk_db_update_buddy_events(struct upk_db *upk_db,
                               int *bpid,
                               int *pid, 
                               int *status);


char *upk_db_created( 
    sqlite3 *pdb
);

int upk_db_changed( 
    sqlite3 *pdb
);

char *upk_db_file_main( void );
char *upk_db_file_misc( void );

void upk_db_file_main_set( char *filename );
void upk_db_file_misc_set( char *filename );

int upk_db_note_exit(
    struct upk_db *upk_db,
    int      status, 
    int      bpid
);

int 
upk_db_open(
    const char     *file, 
    sqlite3       **ppdb
);

void upk_db_api_service_visitor( 
    sqlite3 *pdb, 
    void    (*callback)(),
    void    *context
);

#endif
