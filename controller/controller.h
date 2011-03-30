#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "store/upk_db.h"

struct srvc_fd { 
  /* file descriptor open to buddy, or -1 if no file descriptor yet */
  int fd;
  /* buddy pid, or -1 if this entry is unused */
  int bpid;
  /* the service that this buddy runs on */
  struct upk_srvc srvc;
};

void upk_db_reset_launchcallback( 
    upk_srvc_t  srvc,                                    
    char    *status_desired,
    char    *status_actual
);
void upk_controller_bootstrap   ( sqlite3 *pdb );
void upk_controller_status_fixer( sqlite3 *pdb, struct srvc_fd *fd );
int  upk_controller_socket_init  ( char    *path);

int upk_controller_flush_events(
    struct upk_db *upk_db
);
  
int upk_controller_handle_buddy_status(
    struct upk_db *upk_db,
    int  sock,
    char *msg
);

#define FATAL "buddy-controller: fatal: "
#define ERROR "buddy-controller: error: "

#endif
