
/****************************************************************************
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

#ifndef _UPK_V0_PROTOCOL_STRUCTS_H
#define _UPK_V0_PROTOCOL_STRUCTS_H

/* *******************************************************************************************************************
   |----|----|----|
   ****************************************************************************************************************** */
#define UPK_V0_UPK_REQ_SEQ_START_T_FIELDS \
    upk_msgtype_t       msgtype; \
    upk_msgtype_t       msg_seq_type; \
    uint32_t            msg_seq_count                      /* 0 if open-ended, to be terminated with a
                                                              seq_end msg */


/* 
 * XXX: All bool types will be typecast to uint8_t when serialized (they probably are already on your platform anyway)
 */

/* *******************************************************************************************************************
   |----|-|
   ****************************************************************************************************************** */
#define UPK_V0_UPK_REQ_SEQ_END_T_FIELDS \
    upk_msgtype_t       msgtype; \
    bool                commit                         /* if false, the preceding sequence of requests since the */ \
                                                           /* initiating seq_start will be discarded */

/* *******************************************************************************************************************
   |----|----|<svc_id ...>|----|<action ...>|
   ****************************************************************************************************************** */
#define UPK_V0_REQ_ACTION_T_FIELDS \
    upk_msgtype_t       msgtype; \
    uint32_t            svc_id_len; \
    char                svc_id[UPK_MAX_STRING_LEN]; \
    uint32_t            action_len; \
    char                action[UPK_MAX_STRING_LEN]

/* *******************************************************************************************************************
   |----|----|-|-|----|<svc_id ...>|
   ****************************************************************************************************************** */
#define UPK_V0_REQ_SIGNAL_T_FIELDS \
    upk_msgtype_t       msgtype; \
    upk_signal_t        signal; \
    bool                signal_sid; \
    bool                signal_pgrp; \
    uint32_t            svc_id_len; \
    char                svc_id[UPK_MAX_STRING_LEN]

/* *******************************************************************************************************************
   |----|
   ****************************************************************************************************************** */
#define UPK_V0_REQ_LIST_T_FIELDS \
    upk_msgtype_t       msgtype

/* *******************************************************************************************************************
   |----|----|<svc_id...>|
   ****************************************************************************************************************** */
#define UPK_V0_REQ_STATUS_T_FIELDS \
    upk_msgtype_t       msgtype; \
    uint32_t            svc_id_len; \
    char                svc_id[UPK_MAX_STRING_LEN]; \
    uint32_t            restart_window_seconds

/* 
 * XXX: All bool types will be typecast to uint8_t when serialized (they probably are already on your platform anyway)
 */

/* *******************************************************************************************************************
   |----|-|----|<svc_id...>|
   ****************************************************************************************************************** */
#define UPK_V0_REQ_SUBSCR_T_FIELDS \
    upk_msgtype_t       msgtype; \
    bool                all_svcs;                     /* send a req with all=true to subscribe to everything in */ \
                                                          /* one shot; svc_id will be ignored, and should have len 0 */ \
    uint32_t            svc_id_len; \
    char                svc_id[UPK_MAX_STRING_LEN]

/* 
 * XXX: All bool types will be typecast to uint8_t when serialized (they probably are already on your platform anyway)
 */

/* *******************************************************************************************************************
   |----|-|----|<svc_id ...>|
   ****************************************************************************************************************** */
#define UPK_V0_REQ_UNSUBS_T_FIELDS \
    upk_msgtype_t       msgtype; \
    bool                all_svcs; \
    uint32_t            svc_id_len; \
    char                svc_id[UPK_MAX_STRING_LEN]

/* *******************************************************************************************************************
   |----|
   ****************************************************************************************************************** */
#define UPK_V0_REQ_DISCON_T_FIELDS \
    upk_msgtype_t       msgtype

/* *******************************************************************************************************************
   |----|----|----|
   ****************************************************************************************************************** */
#define UPK_V0_UPK_REPL_SEQ_START_T_FIELDS \
    upk_msgtype_t      msgtype; \
    upk_msgtype_t      msg_seq_type; \
    uint32_t                msg_seq_count

/* *******************************************************************************************************************
   |----|-|
   ****************************************************************************************************************** */
#define UPK_V0_UPK_REPL_SEQ_END_T_FIELDS \
    upk_msgtype_t      msgtype; \
    bool                    commit

/* *******************************************************************************************************************
   replies to signal, and action requests; and informs you if the action was successful. note that if there was a
   protocol-type error while performing the action, that will be reported here if it was an error with the service
   itself; but reported as an ERROR if, for instance, the service didn't exist, or other controller-level error
   occured. |----|-|----|<msg...>|
   ****************************************************************************************************************** */
#define UPK_V0_REPL_RESULT_T_FIELDS \
    upk_msgtype_t      msgtype; \
    bool                    successful; \
    uint32_t                msg_len; \
    char                    msg[UPK_MAX_STRING_LEN]

/* *******************************************************************************************************************
   These will be wrapped in PREAMBLE...COMMIT sequences |----|----|<svc_id...>|
   ****************************************************************************************************************** */
#define UPK_V0_REPL_LISTING_T_FIELDS \
    upk_msgtype_t      msgtype; \
    uint32_t                svc_id_len; \
    char                    svc_id[UPK_MAX_STRING_LEN]

/* *******************************************************************************************************************
   |----|----|<last_action_name...>|----|----|----|----|----|----|----|
   ****************************************************************************************************************** */
#define UPK_V0_SVCINFO_T_FIELDS \
    uint32_t                last_action_time; \
    uint32_t                last_action_status; \
    uint32_t                last_action_name_len; \
    char                    last_action_name[UPK_MAX_STRING_LEN]; \
    uint32_t                last_signal_time; \
    uint32_t                last_signal_status; \
    upk_signal_t            last_signal_name; \
    uint32_t                buddy_pid; \
    uint32_t                proc_pid; \
    upk_state_t             current_state; \
    upk_state_t             prior_state; \
    uint32_t                n_recorded_restarts

/* *******************************************************************************************************************
   |----|<svcinfo ...>|----|<svc_id...|
   ****************************************************************************************************************** */
#define UPK_V0_REPL_SVCINFO_T_FIELDS \
    upk_msgtype_t           msgtype; \
    uint32_t                svcinfo_len;                   /* sizeof struct can be arch dependant; immaterial on unix */ \
                                                           /* domain socket; but to keep this network safe, I'm going */ \
                                                           /* to go ahead and serialize it the same as everything else */ \
    v0_svcinfo_t            svcinfo; \
    uint32_t                svc_id_len; \
    char                    svc_id[UPK_MAX_STRING_LEN]

/* *******************************************************************************************************************
   for when there's really nothing else to say, but you need to assure the client you were paying attention. |----|
   ****************************************************************************************************************** */
#define UPK_V0_REPL_ACK_T_FIELDS \
    upk_msgtype_t           msgtype

/* *******************************************************************************************************************
   |----|-|----|<msg...>|----|<svc_id_len...>|
   ****************************************************************************************************************** */
#define UPK_V0_REPL_ERROR_T_FIELDS \
    upk_msgtype_t           msgtype; \
    upk_errlevel_t          errlevel; \
    upk_errno_t             uerrno; \
    uint32_t                msg_len; \
    char                    msg[UPK_MAX_STRING_LEN]; \
    uint32_t                svc_id_len; \
    char                    svc_id[UPK_MAX_STRING_LEN]

/* *******************************************************************************************************************
   |---| This just serves as an unsolicited reply marker; which is then followed by a repl sequence wrapped up in
   seq_start/seq_end packets.
   ****************************************************************************************************************** */
#define UPK_V0_PUBLICATION_T_FIELDS \
    upk_msgtype_t       msgtype

/* *******************************************************************************************************************
   |---| This just serves as an unsolicited reply marker; which notifies the client that something they subscribed to
   is no longer available for subscription
   ****************************************************************************************************************** */
#define UPK_V0_CANCELATION_T_FIELDS \
    upk_msgtype_t       msgtype



#endif
