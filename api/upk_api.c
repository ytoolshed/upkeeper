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

#define upk_handle_valid( x ) assert( x->magic[3] == UPK_MAGIC[3] )

#define upk_api_handle_to_srvc( pupk_api_p, package_p, service_p, srvc_p ) \
    srvc_p.upk_db  = pupk_api_p->upk_db; \
    srvc_p.package = package_p; \
    srvc_p.service = service_p;

/* 
 * Initialize
 */
int 
upk_api_init(
    struct upk_api *pupk_api
) {
    int rc = 0;

    rc = upk_db_init( &pupk_api->upk_db );

    if( rc == 0 ) {
	pupk_api->magic = UPK_MAGIC;
    }

    return( rc );
}

/* 
 * Set the desired status of a service, which will notify the controller
 * 
 */
int
upk_api_service_desired_status_set(
    struct upk_api *pupk_api,
    char           *package,
    char           *service,
    upk_status_t    upk_status
) {
    int             rc = 0;
    struct upk_srvc srvc;
    const char     *status;

    upk_handle_valid( pupk_api );

    upk_api_handle_to_srvc( pupk_api, package, service, srvc );

    status = upk_db_service_desired_status( &srvc, upk_status );

    if( status == NULL ) {
        return( -1 );
    } 

    upk_db_listener_send_all_signals( pupk_api->upk_db.pdb_misc );

    return( 0 );
}

/* 
 * Retrieve the current setting of the desired state of a service
 */
const char *
upk_api_service_desired_status_get(
    struct upk_api *pupk_api,
    char           *package,
    char           *service
) {
    int             rc = 0;
    struct upk_srvc srvc;
    const char     *status;

    upk_api_handle_to_srvc( pupk_api, package, service, srvc );

    status = upk_db_service_desired_status( &srvc, UPK_STATE_UNKNOWN );

      /* TODO: we might want to return the enum instead */
    return( status );
}

/* 
 * Retrieve the current setting of the actual state of a service
 */
const char *
upk_api_service_actual_status_get(
    struct upk_api *pupk_api,
    char           *package,
    char           *service
) {
    int             rc = 0;
    struct upk_srvc srvc;
    const char     *status;

    upk_api_handle_to_srvc( pupk_api, package, service, srvc );

    status = upk_db_service_actual_status( &srvc, UPK_STATE_UNKNOWN );

    return( status );
}

/* 
 * Define the command line executed by a package/service.
 */
int 
upk_api_service_set(
    struct upk_api *pupk_api,
    char           *package,
    char           *service,
    char           *cmdline
) {
    int             rc = 0;
    struct upk_srvc srvc;
    const char     *status;

    upk_api_handle_to_srvc( pupk_api, package, service, srvc );

    status = upk_db_service_cmdline( &srvc, cmdline );

    if( status == NULL ) {
	rc = -1;
    }

    return( rc );
}

/* 
 * Iterate over all defined services, and on each entry found,
 * call the specified callback like
 *     *callback( upk_api_service_t data, context )
 * where the data struct contains the fields 'package', 'service',
 * 'cmdline' and more.
 */
int 
upk_api_service_visitor(
    struct upk_api *pupk_api,
    void           (*callback)(),
    void            *context
) {
    int rc = 0;

    upk_db_service_visitor( pupk_api->upk_db.pdb, callback, context );

    return( rc );
}
