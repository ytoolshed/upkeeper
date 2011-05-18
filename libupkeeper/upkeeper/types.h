#ifndef _UPK_TYPES_H
#define _UPK_TYPES_H

#include "std_include.h"

#define UPK_MAX_STRING_LEN 2048

typedef enum {
    UPK_STATE_RUNNING=1,
    UPK_STATE_STARTING, 
    UPK_STATE_DOWN,
} upk_state_t;

typedef enum {
    UPK_ERRLVL_ERROR,
} upk_errlevel_t;

typedef enum {
    UPK_SIG_HUP = 1,
    UPK_SIG_INT = 2,
    UPK_SIG_QUIT = 3,
    UPK_SIG_ILL = 4,
    UPK_SIG_TRAP = 5,
    UPK_SIG_ABRT = 6,
    UPK_SIG_BUS = 7,
    UPK_SIG_FPE = 8,
    UPK_SIG_KILL = 9,
    UPK_SIG_USR1 = 10,
    UPK_SIG_SEGV = 11,
    UPK_SIG_USR2 = 12,
    UPK_SIG_PIPE = 13,
    UPK_SIG_ALRM = 14,
    UPK_SIG_TERM = 15,
    UPK_SIG_STKFLT = 16,
    UPK_SIG_CHLD = 17,
    UPK_SIG_CONT = 18,
    UPK_SIG_STOP = 19,
    UPK_SIG_TSTP = 20,
    UPK_SIG_TTIN = 21,
    UPK_SIG_TTOU = 22,
    UPK_SIG_URG = 23,
    UPK_SIG_XCPU = 24,
    UPK_SIG_XFSZ = 25,
    UPK_SIG_VTALRM = 26,
    UPK_SIG_PROF = 27,
    UPK_SIG_WINCH = 28,
    UPK_SIG_IO = 29,
    UPK_SIG_PWR = 30,
    UPK_SIG_SYS = 31,
} upk_signal_name_t;

#endif
