#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "store/upk_db.h"

typedef struct upk_controller_state *upkctl_t;

upkctl_t upk_controller_init(struct upk_db *db);

void upk_db_reset_launchcallback( 
    upk_srvc_t  srvc,                                    
    char    *status_desired,
    char    *status_actual
);
/*void upk_controller_status_fixer( sqlite3 *pdb, struct srvc_fd *fd );*/

int upkctl_flush_events(upkctl_t state);
int upk_controller_handle_buddy_status(
                                       upkctl_t state,
                                       int  sock,
                                       char *msg
                                       );

#define FATAL "buddy-controller: fatal: "
#define ERROR "buddy-controller: error: "

#endif
