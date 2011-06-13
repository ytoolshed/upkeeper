#ifndef _UPK_TYPES_H
#define _UPK_TYPES_H

#include "std_include.h"
#include "v0_protocol_structs.h"

#define UPK_MAX_STRING_LEN 2048

typedef enum {
    UPK_STATE_UNDEFINED,
    UPK_STATE_RUNNING,
    UPK_STATE_STARTING,
    UPK_STATE_STOPPING,
    UPK_STATE_NOT_RUNNING,
    UPK_STATE_SHUTDOWN,
} upk_state_t;

typedef enum {
    UPK_ERRLVL_ERROR,
} upk_errlevel_t;

/* ************************************* */
/* These two items must be kept in sync: */
/* ************************************* */
typedef enum {
    UPK_ERR_UNKNOWN = 0,
    UPK_ERR_UNSUP,
    UPK_ERR_INVALID_PKT,
    UPK_SOCKET_FAILURE,
} upk_error_t;

#define __UPK_ERRORS_ARRAY \
    const unsigned char     __upk_errors[][128] = { \
        "unknown", \
        "unsupported", \
        "invalid packet", \
        "socket failure", \
    }

/* ************************************* */
/* ************************************* */

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
} upk_signal_t;


typedef struct _upk_svclist upk_svclist_t;
struct _upk_svclist {
    unsigned char           svc[UPK_MAX_STRING_LEN];
    upk_svclist_t          *next;
};

typedef struct {
    UPK_V0_SVCINFO_T_FIELDS;
} upk_svcinfo_t;



#define UPKLIST_METANODE(TYPE, NAME) \
    struct { TYPE * head; TYPE * tail; TYPE * prevp; TYPE * nextp; TYPE * tempp; TYPE * thisp; uint32_t count; } * NAME


#define UPKLIST_INIT(TYPE, NAME) \
    UPKLIST_METANODE(TYPE, NAME) = NULL; \
    NAME = calloc(1, sizeof(*NAME))


#define _UPKLIST_NEWNODE(NAME) \
    NAME->tempp = calloc(1,sizeof(*(NAME->tempp)))


#define UPKLIST_APPEND_THIS(NAME) \
    _UPKLIST_NEWNODE(NAME); \
    NAME->tempp->next = NAME->nextp; \
    if(! NAME->nextp ) { NAME->tail = NAME->tempp; } \
    if(! NAME->prevp && NAME->count == 0 ) { NAME->head = NAME->tempp; } \
    if(NAME->thisp) { NAME->thisp->next = NAME->tempp; } \
    ++NAME->count; \
    NAME->thisp = NAME->tempp;


#define UPKLIST_APPEND(NAME) \
    NAME->thisp = NAME->tail; \
    NAME->prevp = (NAME->prevp) ? NAME->prevp : NAME->head; \
    UPKLIST_APPEND_THIS(NAME); \
    NAME->prevp = NULL


#define UPKLIST_PREPEND_THIS(NAME) \
    _UPKLIST_NEWNODE(NAME); \
    NAME->tempp->next = NAME->thisp; \
    if(! NAME->nextp ) { NAME->tail = NAME->tempp; } \
    if(! NAME->prevp ) { NAME->head = NAME->tempp; } else { NAME->prevp->next = NAME->tempp; } \
    ++NAME->count; \
    NAME->thisp = NAME->tempp


#define UPKLIST_PREPEND(NAME) \
    NAME->prevp = NULL; \
    NAME->thisp = NAME->head; \
    UPKLIST_PREPEND_THIS(NAME)

#define _UPKLIST_NEXTNODE(NAME) ( (NAME->thisp) ? NAME->thisp->next : NULL )

#define _UPKLIST_FOREACH_CONTINUE(NAME) \
    NAME->tempp = NULL, NAME->prevp = NAME->thisp, NAME->thisp = NAME->nextp, NAME->nextp = _UPKLIST_NEXTNODE(NAME)


#define _UPKLIST_FOREACH_INIT(NAME) \
    NAME->tempp = NULL, NAME->prevp = NULL, NAME->thisp = NAME->head, NAME->nextp = _UPKLIST_NEXTNODE(NAME)


#define UPKLIST_FOREACH(NAME) \
    for( _UPKLIST_FOREACH_INIT(NAME); NAME->thisp != NULL; _UPKLIST_FOREACH_CONTINUE(NAME) )


#define UPKLIST_SWAP(NAME, A, APREV, B, BPREV) \
    NAME->tempp = calloc(1,sizeof(*NAME->tempp)); \
    APREV->next = B; \
    BPREV->next = A; \
    NAME->tempp->next = A->next; \
    A->next = B->next; \
    B->next = NAME->tempp->next; \
    free(NAME->tempp); NAME->tempp = NULL


#define UPKLIST_UNLINK(NAME) \
    if(NAME->thisp) { \
        if(! NAME->prevp ) { NAME->head = NAME->nextp; } else { NAME->prevp->next = NAME->nextp; } \
        if(! NAME->nextp ) { NAME->tail = NAME->prevp; } \
        free(NAME->thisp); NAME->thisp = NULL; \
        --NAME->count; \
    }


#define UPKLIST_FREE(NAME) \
    UPKLIST_FOREACH(NAME) { \
        UPKLIST_UNLINK(NAME); \
    }\
    if(NAME) { free(NAME); }


/* *********************************** *********************************** */

#define UPKDLIST_METANODE(TYPE, NAME) \
    struct { TYPE * head; TYPE * tail; TYPE * prevp; TYPE * nextp; TYPE * tempp; TYPE * thisp; uint32_t count; } * NAME


#define _UPKDLIST_NEWNODE(NAME) \
    NAME->tempp = calloc(1,sizeof(*(NAME->tempp)))


#define UPKDLIST_INIT(TYPE, NAME) \
    UPKDLIST_METANODE(TYPE, NAME) = NULL; \
    NAME = calloc(1, sizeof(*NAME)); \


#define UPKDLIST_APPEND_THIS(NAME) \
    _UPKDLIST_NEWNODE(NAME); \
    NAME->tempp->next = NAME->nextp; \
    if(NAME->thisp) { NAME->thisp->next = NAME->tempp; NAME->tempp->prev = NAME->thisp; } \
    if(! NAME->nextp ) { NAME->tail = NAME->tempp; } \
    if(! NAME->prevp && NAME->count == 0 ) { NAME->head = NAME->tempp; } \
    ++NAME->count; \
    NAME->thisp = NAME->tempp;


#define UPKDLIST_APPEND(NAME) \
    NAME->thisp = NAME->tail; \
    NAME->prevp = (NAME->thisp) ? NAME->thisp->prev : NAME->head; \
    UPKDLIST_APPEND_THIS(NAME)


#define UPKDLIST_PREPEND_THIS(NAME) \
    _UPKDLIST_NEWNODE(NAME); \
    NAME->tempp->next = NAME->thisp; \
    if(NAME->thisp) {  NAME->tempp->prev = NAME->thisp->prev; NAME->thisp->prev = NAME->tempp; } \
    if(! NAME->nextp ) { NAME->tail = NAME->tempp; } \
    if(! NAME->prevp ) { NAME->head = NAME->tempp; } else { NAME->prevp->next = NAME->tempp; } \
    ++NAME->count; \
    NAME->thisp = NAME->tempp


#define UPKDLIST_PREPEND(NAME) \
    NAME->prevp = NULL; \
    NAME->thisp = NAME->head; \
    UPKDLIST_PREPEND_THIS(NAME)

#define _UPKDLIST_NEXTNODE(NAME) ( (NAME->thisp) ? NAME->thisp->next : NULL )
#define _UPKDLIST_PREVNODE(NAME) ( (NAME->thisp) ? NAME->thisp->prev : NULL )

#define _UPKDLIST_FOREACH_CONTINUE(NAME) \
    NAME->tempp = NULL, NAME->prevp = NAME->thisp, NAME->thisp = NAME->nextp, \
    NAME->nextp = _UPKDLIST_NEXTNODE(NAME), NAME->prevp = _UPKDLIST_PREVNODE(NAME)

#define _UPKDLIST_FOREACH_INIT(NAME) \
    NAME->tempp = NULL, NAME->prevp = NULL, NAME->thisp = NAME->head, NAME->nextp = _UPKDLIST_NEXTNODE(NAME)


#define UPKDLIST_FOREACH(NAME) \
    for( _UPKDLIST_FOREACH_INIT(NAME); NAME->thisp != NULL; _UPKDLIST_FOREACH_CONTINUE(NAME) )

#define _UPKDLIST_FOREACH_R_CONTINUE(NAME) \
    NAME->tempp = NULL, NAME->nextp = NAME->thisp, NAME->thisp = NAME->prevp, \
    NAME->prevp = _UPKDLIST_PREVNODE(NAME), NAME->nextp = _UPKDLIST_NEXTNODE(NAME)

#define _UPKDLIST_FOREACH_R_INIT(NAME) \
    NAME->tempp = NULL, NAME->nextp = NULL, NAME->thisp = NAME->tail, NAME->prevp = _UPKDLIST_PREVNODE(NAME)

#define UPKDLIST_FOREACH_REVERSE(NAME) \
    for( _UPKDLIST_FOREACH_R_INIT(NAME); NAME->thisp != NULL; _UPKDLIST_FOREACH_R_CONTINUE(NAME) )


#define UPKDLIST_SWAP(NAME, A, B) \
    NAME->tempp = calloc(1,sizeof(*NAME->tempp)); \
    A->prev->next = B; \
    B->prev->next = A; \
    NAME->tempp->next = A->next; \
    NAME->tempp->prev = A->prev; \
    A->next = B->next; \
    A->prev = B->prev; \
    B->next = NAME->tempp->next; \
    B->prev = NAME->tempp->prev; \
    free(NAME->tempp); NAME->tempp = NULL


#define UPKDLIST_UNLINK(NAME) \
    if(NAME->thisp) { \
        if(! NAME->prevp ) { NAME->head = NAME->nextp; } else { NAME->prevp->next = NAME->nextp; } \
        if(! NAME->nextp ) { NAME->tail = NAME->prevp; } else { NAME->nextp->prev = NAME->prevp; }  \
        free(NAME->thisp); NAME->thisp = NULL; \
        --NAME->count; \
    }


#define UPKDLIST_FREE(NAME) \
    UPKDLIST_FOREACH(NAME) { \
        UPKDLIST_UNLINK(NAME); \
    }\
    if(NAME) { free(NAME); }


#endif
