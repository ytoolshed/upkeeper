#ifndef _UPK_TYPES_H
#define _UPK_TYPES_H

#include "std_include.h"

#define UPK_MAX_STRING_LEN 2048

typedef uint8_t         upk_state;

/* XXX: don't specify more than 31 primary, and 31 secondary field
   identifiers either here, or in the struct that contains the info;
   that would be bad. */

typedef enum {
    LAST_ACTION_TIME = 1,                                  /* 1 */
    LAST_ACTION_STATUS = 2,                                /* 2 */
    LAST_ACTION_NAME = 4,                                  /* 3 */
    LAST_SIGNAL_TIME = 8,                                  /* 4 */
    LAST_SIGNAL_STATUS = 16,                               /* 5 */
    LAST_SIGNAL_NAME = 32,                                 /* 6 */
    BUDDY_PID = 64,                                        /* 7 */
    PROC_PID = 128,                                        /* 8 */
    CURRENT_STATE = 256,                                   /* 9 */
    PRIOR_STATE = 512,                                     /* 10 */
} pfieldmask_t;


typedef enum {
    NO_SECONDARY_FIELDS_YET = 0,
} sfieldmask_t;

typedef uint8_t         upk_errlevel_t;

typedef struct {
    time_t                  last_action_time;
    siginfo_t               last_action_status;
    char                   *last_action_name;
    time_t                  last_signal_time;
    siginfo_t               last_signal_status;
    char                   *last_signal_name;              /* XXX: use name rather than number; to void confusion */
    pid_t                   buddy_pid;
    pid_t                   proc_pid;
    upk_state               current_state;
    upk_state               prior_state;
} upk_svc_info_t;


#endif
