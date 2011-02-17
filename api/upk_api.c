/* 
 * upkeeper's external API
 */
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

#include "store/upk_db.h"
#include "api/upk_api.h"

/* 
 * Initialize
 */
int 
upk_api_init(
    struct upk_api *pupk_api
) {
    int rc = 0;

    rc = upk_db_init( &pupk_api->upk_db );

    return( rc );
}

/* 
 * 
 */
int
upk_api_service_desired_status_set(
    struct upk_api *pupk_api,
    char           *package,
    char           *service,
    upk_status_t    upk_status
) {
    int rc = 0;

    return( rc );
}

/* 
 * 
 */
int 
upk_api_service_desired_status_get(
    struct upk_api *pupk_api,
    char           *package,
    char           *service
) {
    int rc = 0;

    return( rc );
}

/* 
 * 
 */
int 
upk_api_service_actual_status_get(
    struct upk_api *pupk_api,
    char           *package,
    char           *service
) {
    int rc = 0;

    return( rc );
}

/* 
 * 
 */
int 
upk_api_service_actual_status_set(
    struct upk_api *pupk_api,
    char           *package,
    char           *service,
    upk_status_t    upk_status
) {
    int rc = 0;

    return( rc );
}

/* 
 * 
 */
int 
upk_api_service_set(
    struct upk_api *pupk_api,
    char           *package,
    char           *service,
    char           *cmd,
    char           *cmd_params[]
) {
    int rc = 0;

    return( rc );
}

/* 
 * 
 */
int 
upk_api_service_visitor(
    struct upk_api *pupk_api,
    char           *package,
    char           *service,
    void           (*callback)(),
    void            *context
) {
    int rc = 0;

    return( rc );
}

