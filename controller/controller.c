
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "../store/upk_db.h"
#include <unistd.h>
#include "controller.h"

/* 
 * Bring a service inline with the desired status
 */
void upk_controller_status_fixer_callback( 
    upk_srvc_t  srvc,                                    
    char    *status_desired,
    char    *status_actual
) {
    if( strcmp( status_desired, status_actual ) == 0 ) {
	return;
    }

    if( strcmp( status_desired, upk_states[ UPK_STATUS_VALUE_START ] ) == 0 ) {
	/* service needs to be started */
	printf("** STARTING %s/%s\n", srvc->package, srvc->service );
    } else {
	/* service needs to be stopped */
	printf("** STOPPING %s/%s\n", srvc->package, srvc->service );
    }

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

