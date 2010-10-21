/* Upkeeper constants */

#define UPK_OK                     0
#define UPK_ERROR_SERVICE_UNKNOWN -1
#define UPK_ERROR_PACKAGE_UNKNOWN -2
#define UPK_ERROR_INTERNAL        -3
#define UPK_DB_ERROR              -4

#define UPK_STATUS_ACTUAL  0
#define UPK_STATUS_DESIRED 1

#define UPK_STATUS_VALUE_START "start"
#define UPK_STATUS_VALUE_STOP  "stop"

/* Prototypes */

#include <sqlite3.h>

int upk_db_init( const char *file, sqlite3 **ppdb );

int upk_db_close(
    sqlite3 *pdb 
);

typedef enum { 
  UP,         /* Set by buddy upon sucessful fork() */
  DOWN,       /* Set on buddy resumption after sigchld   */
  ALIVE       /* Set by */
} process_states;

const char *upk_db_service_actual_status( 
    sqlite3 *pdb, 
    const char    *package, 
    const char    *service,
    const char    *new_status
);

const char *upk_db_service_desired_status( 
    sqlite3 *pdb, 
    const char    *package, 
    const char    *service,
    const char    *new_status
);

char *upk_db_time_now_mstring( void );

char *upk_db_exec_single( 
    sqlite3  *pdb, 
    const char     *sql
);

int upk_db_service_find_or_create( 
    sqlite3 *pdb, 
    const char    *package, 
    const char    *service
);

int upk_db_service_find( 
    sqlite3 *pdb, 
    const char    *package, 
    const char    *service
);

void upk_db_status_checker( 
    sqlite3 *pdb, 
    void (*callback)()
);

void _upk_db_status_checker_testcallback( 
    sqlite3 *pdb, 
    const char    *package, 
    const char    *service,
    const char    *status_desired,
    const char    *status_actual
);

void upk_db_status_checker_launchcallback( 
    sqlite3 *pdb, 
    const char    *package, 
    const char    *service,
    const char    *status_desired,
    const char    *status_actual
);

int upk_test_is(
    int is,
    int should
);

int upk_test_eq(
    const char *is,
    const char *should
);
int upk_test_isnt(
    int is,
    int shouldnt 
);

const char *upk_db_service_run( 
    sqlite3 *pdb, 
    const char    *package, 
    const char    *service,
    const char    *cmdline,
    int   pid
);

int 
upk_db_listener_add(
    sqlite3    *pdb, 
    const char *component, 
    int         pid
);

int 
upk_db_listener_remove(
    sqlite3    *pdb, 
    const char *component
);

void upk_db_listener_checker( 
    sqlite3 *pdb, 
    char    *component,
    void (*callback)()
);
