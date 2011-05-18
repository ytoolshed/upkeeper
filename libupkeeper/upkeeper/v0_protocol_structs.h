#ifndef _UPK_V0_PROTOCOL_STRUCTS_H
#define _UPK_V0_PROTOCOL_STRUCTS_H

/* *******************************************************************************************************************
 * |----|----|----|
 * ****************************************************************************************************************** */
#define UPK_V0_REQ_SEQ_START_T_FIELDS \
    upk_req_msgtype_t       msgtype; \
    upk_req_msgtype_t       msg_seq_type; \
    uint32_t                msg_seq_count                  /* 0 if open-ended, to be terminated with a seq_end msg */


/* 
 * XXX: All bool types will be typecast to uint8_t when serialized (they probably are already on your platform anyway)
 */

/* *******************************************************************************************************************
 * |----|-|
 * ****************************************************************************************************************** */
#define UPK_V0_REQ_SEQ_END_T_FIELDS \
    upk_req_msgtype_t       msgtype; \
    bool                    commit                         /* if false, the preceding sequence of requests since the */ \
                                                           /* initiating seq_start will be discarded */

/* *******************************************************************************************************************
 * |----|----|<svc_id ...>|----|<action ...>|
 * ****************************************************************************************************************** */
#define UPK_V0_ACTION_REQ_T_FIELDS \
    upk_req_msgtype_t       msgtype; \
    uint32_t                svc_id_len; \
    char                   *svc_id; \
    uint32_t                action_len; \
    char                   *action

/* *******************************************************************************************************************
 * |----|-|-|-|----|<svc_id ...>|
 * ****************************************************************************************************************** */
#define UPK_V0_SIGNAL_REQ_T_FIELDS \
    upk_req_msgtype_t       msgtype; \
    uint8_t                 signal; \
    bool                    signal_sid; \
    bool                    signal_pgrp; \
    uint32_t                svc_id_len; \
    char                   *svc_id

/* *******************************************************************************************************************
 * |----|
 * ****************************************************************************************************************** */
#define UPK_V0_LIST_REQ_T_FIELDS \
    upk_req_msgtype_t       msgtype

/* *******************************************************************************************************************
 * |----|----|<svc_id...>|
 * ****************************************************************************************************************** */
#define UPK_V0_STATUS_REQ_T_FIELDS \
    upk_req_msgtype_t       msgtype; \
    uint32_t                svc_id_len; \
    char                   *svc_id

/* 
 * XXX: All bool types will be typecast to uint8_t when serialized (they probably are already on your platform anyway)
 */

/* *******************************************************************************************************************
 * |----|-|----|<svc_id...>|
 * ****************************************************************************************************************** */
#define UPK_V0_SUBSCRIBE_REQ_T_FIELDS \
    upk_req_msgtype_t       msgtype; \
    bool                    all_svcs;                     /* send a req with all=true to subscribe to everything in */ \
                                                          /* one shot; svc_id will be ignored, and should have len 0 */ \
    uint32_t                svc_id_len; \
    char                   *svc_id

/* 
 * XXX: All bool types will be typecast to uint8_t when serialized (they probably are already on your platform anyway)
 */

/* *******************************************************************************************************************
 * |----|-|----|<svc_id ...>|
 * ****************************************************************************************************************** */
#define UPK_V0_UNSUBSCRIBE_REQ_T_FIELDS \
    upk_req_msgtype_t       msgtype; \
    bool                    all_svcs; \
    uint32_t                svc_id_len; \
    char                   *svc_id

/* *******************************************************************************************************************
 * |----|
 * ****************************************************************************************************************** */
#define UPK_V0_DISCONNECT_REQ_T_FIELDS \
    upk_req_msgtype_t       msgtype

/* *******************************************************************************************************************
 * |----|----|----|
 * ****************************************************************************************************************** */
#define UPK_V0_REPL_SEQ_START_T_FIELDS \
    upk_repl_msgtype_t      msgtype; \
    upk_repl_msgtype_t      msg_seq_type; \
    uint32_t                msg_seq_count

/* *******************************************************************************************************************
 * |----|-|
 * ****************************************************************************************************************** */
#define UPK_V0_REPL_SEQ_END_T_FIELDS \
    upk_repl_msgtype_t      msgtype; \
    bool                    commit

/* *******************************************************************************************************************
 * replies to signal, and action requests; and informs you if the action was successful. note that if there was a
 * protocol-type error while performing the action, that will be reported here if it was an error with the service
 * itself; but reported as an ERROR if, for instance, the service didn't exist, or other controller-level error
 * occured. |----|-|----|<msg...>|
 * ****************************************************************************************************************** */
#define UPK_V0_RESULT_REPL_T_FIELDS \
    upk_repl_msgtype_t      msgtype; \
    bool                    successful; \
    uint32_t                msg_len; \
    char                   *msg

/* *******************************************************************************************************************
 * These will be wrapped in PREAMBLE...COMMIT sequences |----|----|<svc_id...>|
 * ****************************************************************************************************************** */
#define UPK_V0_LISTING_REPL_T_FIELDS \
    upk_repl_msgtype_t      msgtype; \
    uint32_t                svc_id_len; \
    char                   *svc_id

/* *******************************************************************************************************************
 * |----|----|<last_action_name...>|----|----|----|----|----|----|----|
 * ****************************************************************************************************************** */
#define UPK_V0_SVCINFO_T_FIELDS \
    uint32_t                last_action_time; \
    uint32_t                last_action_status; \
    uint32_t                last_action_name_len; \
    char                   *last_action_name; \
    uint32_t                last_signal_time; \
    uint32_t                last_signal_status; \
    upk_signal_name_t       last_signal_name; \
    uint32_t                buddy_pid; \
    uint32_t                proc_pid; \
    upk_state_t             current_state; \
    upk_state_t             prior_state

/* *******************************************************************************************************************
 * |----|<svcinfo ...>|----|<svc_id...|
 * ****************************************************************************************************************** */
#define UPK_V0_SVCINFO_REPL_T_FIELDS \
    upk_repl_msgtype_t      msgtype; \
    uint32_t                svcinfo_len;                   /* sizeof struct can be arch dependant; immaterial on unix */ \
                                                           /* domain socket; but to keep this network safe, I'm going */ \
                                                           /* to go ahead and serialize it the same as everything else */ \
    v0_svcinfo_t            svcinfo; \
    uint32_t                svc_id_len; \
    char                   *svc_id

/* *******************************************************************************************************************
 * for when there's really nothing else to say, but you need to assure the client you were paying attention. |----|
 * ****************************************************************************************************************** */
#define UPK_V0_ACK_REPL_T_FIELDS \
    upk_repl_msgtype_t      msgtype

/* *******************************************************************************************************************
 * |----|-|----|<msg...>|----|<svc_id_len...>|
 * ****************************************************************************************************************** */
#define UPK_V0_ERROR_REPL_T_FIELDS \
    upk_repl_msgtype_t      msgtype; \
    upk_errlevel_t          errlevel; \
    uint32_t                msg_len; \
    char                   *msg; \
    uint32_t                svc_id_len; \
    char                   *svc_id

/* *******************************************************************************************************************
 * |---| This just serves as an unsolicited reply marker; which is then followed by a repl sequence wrapped up in
 * seq_start/seq_end packets.
 * ****************************************************************************************************************** */
#define UPK_V0_PUBLICATION_T_FIELDS \
    upk_pub_msgtype_t       msgtype

/* *******************************************************************************************************************
 * |---| This just serves as an unsolicited reply marker; which notifies the client that something they subscribed to
 * is no longer available for subscription
 * ****************************************************************************************************************** */
#define UPK_V0_CANCELATION_T_FIELDS \
    upk_pub_msgtype_t       msgtype



#endif
