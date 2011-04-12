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
/* Prototypes */

#include <sqlite3.h>

typedef struct upk_db {
    sqlite3 *pdb;
    sqlite3 *pdb_misc;
} upk_db_t;


typedef struct upk_srvc {
  struct upk_db upk_db;
  char         *package;
  char         *service;
} *upk_srvc_t;


int upk_db_init( struct upk_db *pupk_db );

int upk_db_exit( struct upk_db *pupk_db );

const char *upk_db_service_actual_status( 
    upk_srvc_t srvc,
    upk_state  state
);

/* getter/setter. */
const char *upk_db_service_desired_status( 
    upk_srvc_t srvc,
    upk_state  state
);

/* hmm */
char *upk_db_time_now_mstring( void );

/* speak to memory allocation ? */
char *upk_db_exec_single( 
    sqlite3  *pdb, 
    const char     *sql
);

int upk_db_service_find_or_create( upk_srvc_t );
int upk_db_service_find( upk_srvc_t );
typedef void (*upk_db_visitor_t)(void *context,
				 upk_srvc_t srvc,                                    
				 const char    *status_desired,
				 const char    *status_actual);

void upk_db_status_visitor( 
    sqlite3 *pdb, 
    upk_db_visitor_t callback,
    void  *context
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

typedef void (*upk_db_listener_t)(const char *component,
				  int pid,
				  int signal);
void upk_db_listener_visitor( 
    sqlite3 *pdb, 
    upk_db_listener_t callback;
);

void upk_db_listener_send_all_signals( struct upk_db *db );
void upk_db_listener_remove_dead(sqlite3    *pdb);

void upk_db_clear(struct upk_db *upk_db);

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
int upk_db_buddy_down(struct upk_db *upk_db, int bpid);

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

const char *upk_db_file_main( void );
const char *upk_db_file_misc( void );

void upk_db_file_main_set( const char *filename );
void upk_db_file_misc_set( const char *filename );

int upk_db_note_exit(
    struct upk_db *upk_db,
    int      status, 
    int      bpid
);

void upk_db_api_service_visitor( 
    sqlite3 *pdb, 
    void    (*callback)(),
    void    *context
);

#endif
