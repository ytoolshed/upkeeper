/* Upkeeper constants */

#define UPK_OK                     0
#define UPK_ERROR_SERVICE_UNKNOWN -1
#define UPK_ERROR_PACKAGE_UNKNOWN -2
#define UPK_ERROR_INTERNAL        -3
#define UPK_DB_ERROR              -4

#define UPK_STATUS_ACTUAL  0
#define UPK_STATUS_DESIRED 1

/* Prototypes */

#include <sqlite3.h>

int upk_db_init(
    char     *file, 
    sqlite3 **ppdb 
);

char *upk_db_service_actual_status( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service,
    char    *new_status
);

char *upk_db_service_desired_status( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service,
    char    *new_status
);

char *upk_db_time_now_mstring( void );

char *upk_db_exec_single( 
    sqlite3  *pdb, 
    char     *sql
);

int upk_db_service_find_or_create( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service
);

int upk_db_service_find( 
    sqlite3 *pdb, 
    char    *package, 
    char    *service
);

int upk_test_is(
    int is,
    int should
);

int upk_test_eq(
    char *is,
    char *should
);
