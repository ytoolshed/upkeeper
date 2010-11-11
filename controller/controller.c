
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "../store/upk_db.h"
#include <unistd.h>
#include "controller.h"

extern int DEBUG;

/* 
 * Bring a service inline with the desired status
 */
void upk_controller_status_fixer_callback( 
    upk_srvc_t  srvc,                                    
    char    *status_desired,
    char    *status_actual
) {
    const char *cmdline;
    char       *cmdline_alloc;

    if( strcmp( status_desired, status_actual ) == 0 ) {
	return; /* We're good */
    }

    cmdline = upk_db_service_cmdline( srvc, NULL );

    if( cmdline == NULL ) {
        if( DEBUG ) {
	    printf("** ERROR no cmdline for %s/%s\n", 
                   srvc->package, srvc->service );
        }
        return;
    }

      /* We got a static buffer which is going to be overridden with the
         next sql call, allocate memory */
    cmdline_alloc = strdup( cmdline );

    if( strcmp( status_desired, upk_states[ UPK_STATUS_VALUE_START ] ) == 0 ) {
	/* service needs to be started */
        if( DEBUG ) {
	    printf("** STARTING %s/%s/%s\n", srvc->package, srvc->service,
                                             cmdline_alloc );
        }
        upk_buddy_start_async( srvc, cmdline_alloc, NULL );
    } else {
        if( DEBUG ) {
	    printf("** STOPPING %s/%s/%s\n", srvc->package, srvc->service,
                                             cmdline_alloc );
        }
        upk_buddy_stop( srvc );
    }

    free( cmdline_alloc );
    return;
}

/* 
 * Bring all services inline with the desired status
 */
void upk_controller_status_fixer( 
    sqlite3 *pdb
) {
    upk_db_status_visitor( pdb, upk_controller_status_fixer_callback );
}

