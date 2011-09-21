/* ***************************************************************************
 * Copyright (c) 2011 Yahoo! Inc. All rights reserved. Licensed under the
 * Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License
 * at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable
 * law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 * See accompanying LICENSE file. 
 ************************************************************************** */

#ifndef _UPK_TYPES_H
#define _UPK_TYPES_H

/**
  @file
  @brief types common throughout libupkeeper.

 Types used commonly, and (ideally) not more suitably defined elsewhere. 
 */

#include "upk_std_include.h"
#include "upk_v0_protocol_structs.h"
#include "upk_uuid.h"

#define UPK_MAX_STRING_LEN 2048                            /*!< @brief longest allowable string (other than paths). */
#define UPK_MAX_PATH_LEN 8192                              /*!< @brief longest allowable path. */

/**
  @brief run states to report to clients.

  The current state of a monitored service 
 */
typedef enum {
    UPK_STATE_UNDEFINED,                                   /*!< unknown/undefined; probably an error */
    UPK_STATE_RUNNING,                                     /*!< the service is running */
    UPK_STATE_STOPPED,                                     /*!< the service is stopped */
    UPK_STATE_SHUTDOWN,                                    /*!< the service is stopped, and its buddy is not running */
} upk_state_t;

/** 
  @brief signal names.

  List of signal names, so that the platform's signal numbering is no longer significant for data storage, or
  communication with controller 
 */
typedef enum {
    UPK_SIG_HUP = 1,                                       /*!< hup */
    UPK_SIG_INT = 2,                                       /*!< int */
    UPK_SIG_QUIT = 3,                                      /*!< quit */
    UPK_SIG_ILL = 4,                                       /*!< ill */
    UPK_SIG_TRAP = 5,                                      /*!< trap */
    UPK_SIG_ABRT = 6,                                      /*!< abrt */
    UPK_SIG_BUS = 7,                                       /*!< bus */
    UPK_SIG_FPE = 8,                                       /*!< fpe */
    UPK_SIG_KILL = 9,                                      /*!< kill */
    UPK_SIG_USR1 = 10,                                     /*!< usr1 */
    UPK_SIG_SEGV = 11,                                     /*!< segv */
    UPK_SIG_USR2 = 12,                                     /*!< usr2 */
    UPK_SIG_PIPE = 13,                                     /*!< pipe */
    UPK_SIG_ALRM = 14,                                     /*!< alrm */
    UPK_SIG_TERM = 15,                                     /*!< term */
    UPK_SIG_STKFLT = 16,                                   /*!< stkflt */
    UPK_SIG_CHLD = 17,                                     /*!< chld */
    UPK_SIG_CONT = 18,                                     /*!< cont */
    UPK_SIG_STOP = 19,                                     /*!< stop */
    UPK_SIG_TSTP = 20,                                     /*!< tstp */
    UPK_SIG_TTIN = 21,                                     /*!< ttin */
    UPK_SIG_TTOU = 22,                                     /*!< ttou */
    UPK_SIG_URG = 23,                                      /*!< urg */
    UPK_SIG_XCPU = 24,                                     /*!< xcpu */
    UPK_SIG_XFSZ = 25,                                     /*!< xfsz */
    UPK_SIG_VTALRM = 26,                                   /*!< vtalrm */
    UPK_SIG_PROF = 27,                                     /*!< prof */
    UPK_SIG_WINCH = 28,                                    /*!< winch */
    UPK_SIG_IO = 29,                                       /*!< io */
    UPK_SIG_PWR = 30,                                      /*!< pwr */
    UPK_SIG_SYS = 31,                                      /*!< sys */
} upk_signal_t;

/**
 @brief linked list of service identifiers.
 */
typedef struct _upk_svcid upk_svcid_t;

/**
 @brief linked list of service identifiers.
 */
struct _upk_svcid {
    char                    pkg[UPK_MAX_STRING_LEN];       /*!< service package-prefix */
    char                    name[UPK_MAX_STRING_LEN];      /*!< service name */
    upk_uuid_t              uuid;                          /*!< not sure I need this here; but just in case for now */
    upk_svcid_t            *next;                          /*!< next, for use in lists */
};

/**
  @brief linked list of custom action scripts.
  */
typedef struct _upk_cust_actscr_list upk_cust_actscr_list_t;

/**
  @brief linked list of custom action scripts.
  */
struct _upk_cust_actscr_list {
    char                    name[UPK_MAX_STRING_LEN];      /*!< name of custom action */
    char                   *script;                        /*!< script to run for action */
    upk_cust_actscr_list_t *next;                          /*!< next, for use in lists */
};


/**
  @brief see definition in *protocol_fields.h.
  */
typedef struct _upk_svcinfo {
    UPK_V0_SVCINFO_T_FIELDS;
} upk_svcinfo_t;


/**
  @brief definition of metanode for a given type.

  this macro contains a pionter; so if you want to use it to create a typedef, it would look something like:

  UPKLIST_METANODE(my_listtype_t, listtype_metanode_p), listtype_metanode_t;

  or similar.
  */


#define UPKLIST_METANODE(TYPE, NAME) \
    struct { TYPE * head; TYPE * tail; TYPE * prevp; TYPE * nextp; TYPE * tempp; TYPE * thisp; uint32_t count; void *userdata; void (*userdata_free_func)(void *); } * NAME

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

#define UPKLIST_HEAD(NAME) \
    do { UPKLIST_FOREACH(NAME) { break; } } while(0)

#define UPKLIST_SWAP(NAME, A, APREV, B, BPREV) \
    NAME->tempp = calloc(1,sizeof(*NAME->tempp)); \
    APREV->next = B; \
    BPREV->next = A; \
    NAME->tempp->next = A->next; \
    A->next = B->next; \
    B->next = NAME->tempp->next; \
    free(NAME->tempp); NAME->tempp = NULL


#define UPKLIST_UNLINK(NAME) \
    do { \
        if(NAME->thisp) { \
            if(! NAME->prevp ) { NAME->head = NAME->nextp; } else { NAME->prevp->next = NAME->nextp; } \
            if(! NAME->nextp ) { NAME->tail = NAME->prevp; } \
            free(NAME->thisp); NAME->thisp = NULL; \
            --NAME->count; \
        } \
    } while(0)


#define UPKLIST_FREE(NAME) \
    do { \
        if(NAME) { \
            UPKLIST_FOREACH(NAME) { \
                UPKLIST_UNLINK(NAME); \
            }\
            if(NAME->userdata && NAME->userdata_free_func) { NAME->userdata_free_func(NAME->userdata); } \
            free(NAME); \
        } \
    } while(0)

#define UPKDLIST_METANODE(TYPE, NAME) \
    struct { TYPE * head; TYPE * tail; TYPE * prevp; TYPE * nextp; TYPE * tempp; TYPE * thisp; uint32_t count; void *userdata; void (*userdata_free_func)(void *); } * NAME


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

#define UPKDLIST_HEAD(NAME) \
    do { UPKDLIST_FOREACH(NAME) { break; } } while(0)

#define _UPKDLIST_FOREACH_R_CONTINUE(NAME) \
    NAME->tempp = NULL, NAME->nextp = NAME->thisp, NAME->thisp = NAME->prevp, \
    NAME->prevp = _UPKDLIST_PREVNODE(NAME), NAME->nextp = _UPKDLIST_NEXTNODE(NAME)

#define _UPKDLIST_FOREACH_R_INIT(NAME) \
    NAME->tempp = NULL, NAME->nextp = NULL, NAME->thisp = NAME->tail, NAME->prevp = _UPKDLIST_PREVNODE(NAME)

#define UPKDLIST_FOREACH_REVERSE(NAME) \
    for( _UPKDLIST_FOREACH_R_INIT(NAME); NAME->thisp != NULL; _UPKDLIST_FOREACH_R_CONTINUE(NAME) )

#define UPKDLIST_TAIL(NAME) \
    do { UPKDLIST_FOREACH_REVERSE(NAME) { break; } } while(0)

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
    do{ \
        if(NAME->thisp) { \
            if(! NAME->prevp ) { NAME->head = NAME->nextp; } else { NAME->prevp->next = NAME->nextp; } \
            if(! NAME->nextp ) { NAME->tail = NAME->prevp; } else { NAME->nextp->prev = NAME->prevp; }  \
            free(NAME->thisp); NAME->thisp = NULL; \
            --NAME->count; \
        } \
    } while(0)

#define UPKDLIST_FREE(NAME) \
    do { \
        if(NAME) { \
            UPKDLIST_FOREACH(NAME) { \
                UPKDLIST_UNLINK(NAME); \
            }\
            if(NAME->userdata && NAME->userdata_free_func) { NAME->userdata_free_func(NAME->userdata); } \
            free(NAME); \
        } \
    } while(0)


typedef                 UPKLIST_METANODE(upk_svcid_t, _upk_svcid_meta_p), upk_svcid_meta_t;
typedef                 UPKLIST_METANODE(upk_cust_actscr_list_t, _upk_cust_actscr_meta_p), upk_cust_actscr_meta_t;


#endif
