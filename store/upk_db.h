/* Upkeeper constants */

#define UPK_OK                     0
#define UPK_ERROR_SERVICE_UNKNOWN -1
#define UPK_ERROR_PACKAGE_UNKNOWN -2
#define UPK_ERROR_INTERNAL        -3
#define UPK_DB_ERROR              -4

/* Prototypes */

#include <sqlite3.h>

int upk_db_init( const char *file, sqlite3 **ppdb );

int upk_db_close(
    sqlite3 *pdb 
);

#ifdef __UPK_DB_C
const char *upk_states[] = { "unknown", "start", "stop",  "invalid" };
#else
extern const char *upk_states[];
#endif

typedef enum { 
  UPK_STATUS_VALUE_UNKNOWN = 0,    
  UPK_STATUS_VALUE_START   = 1,
  UPK_STATUS_VALUE_STOP    = 2,
  UPK_STATUS_VALUE_EXITED  = 3,
  UPK_STATUS_VALUE_INVALID = 4,
  UPK_STATUS_VALUE_MAX     = 5

} upk_state;

typedef enum {
  UPK_STATUS_ACTUAL  = 0,
  UPK_STATUS_DESIRED
} upk_status_t;



typedef struct upk_srvc {
  sqlite3 *pdb;
  char    *package;
  char    *service;
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

void upk_db_status_checker( 
    sqlite3 *pdb, 
    void (*callback)()
);

void _upk_db_status_checker_testcallback( 
    upk_srvc_t    srvc,                                         
    const char    *status_desired,
    const char    *status_actual
);

void upk_db_status_checker_launchcallback( 
    upk_srvc_t    srvc,                                         
    const char    *status_desired,
    const char    *status_actual
);

int upk_test_is(
    int is,
    int should,
    const char *msg
);

int upk_test_eq(
    const char *is,
    const char *should
);
int upk_test_isnt(
    int is,
    int shouldnt ,
    const char *msg
);

const char *upk_db_service_run( 
    upk_srvc_t    srvc,                                         
    const char    *cmdline,
    int   pid,
    int  bpid
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

void upk_db_listener_checker( 
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
    sqlite3    *pdb
);

const char *upk_db_service_cmdline( 
                                   upk_srvc_t svc,
                                   const char *cmdline
);

int upk_db_service_pid( 
                                   upk_srvc_t svc,
                                   int         pid
);
