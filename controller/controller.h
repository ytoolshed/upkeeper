#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "store/upk_db.h"

typedef struct upk_controller_state *upkctl_t;


/* create a controller object, given a database
 */
upkctl_t upk_controller_init(struct upk_db *db);

/* free resources attached to a controller */
void     upk_controller_free(upkctl_t);

/* flush queued events to the database */
int      upkctl_flush_events(upkctl_t state);

/* bring all services into desired state in db */
void     upkctl_status_fixer( upkctl_t ctl );


/* handle a message from a buddy on a socket */
int       upkctl_handle_buddy_status(
                               upkctl_t state,
                               int  sock,
                               char *msg
                               );
/* populate an fd_set and maxfd for passing to 
 * select, based on current state of controller */
void
upkctl_build_fdset(upkctl_t state, fd_set *rfds, int *maxfd);

int 
upkctl_scan_directory(upkctl_t state);

#define FATAL "buddy-controller: fatal: "
#define ERROR "buddy-controller: error: "

#endif
