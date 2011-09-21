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

#ifndef _UPK_V0_PROTOCOL_H
#define _UPK_V0_PROTOCOL_H

#include "upk_protocol.h"
#include "upk_v0_protocol_structs.h"


/* *******************************************************************************************************************
 * fields defined in v0_protocol_structs.h; check there for info
 * ****************************************************************************************************************** */
typedef struct {
    UPK_V0_UPK_REQ_SEQ_START_T_FIELDS;
} v0_req_seq_start_t;

typedef struct {
    UPK_V0_UPK_REQ_SEQ_END_T_FIELDS;
} v0_req_seq_end_t;

typedef struct {
    UPK_V0_REQ_ACTION_T_FIELDS;
} v0_req_action_t;

typedef struct {
    UPK_V0_REQ_SIGNAL_T_FIELDS;
} v0_req_signal_t;

typedef struct {
    UPK_V0_REQ_LIST_T_FIELDS;
} v0_req_list_t;

typedef struct {
    UPK_V0_REQ_STATUS_T_FIELDS;
} v0_req_status_t;

typedef struct {
    UPK_V0_REQ_SUBSCR_T_FIELDS;
} v0_req_subscribe_t;

typedef struct {
    UPK_V0_REQ_UNSUBS_T_FIELDS;
} v0_req_unsubscribe_t;

typedef struct {
    UPK_V0_REQ_DISCON_T_FIELDS;
} v0_req_disconnect_t;

typedef struct {
    UPK_V0_UPK_REPL_SEQ_START_T_FIELDS;
} v0_repl_seq_start_t;

typedef struct {
    UPK_V0_UPK_REPL_SEQ_END_T_FIELDS;
} v0_repl_seq_end_t;

typedef struct {
    UPK_V0_REPL_RESULT_T_FIELDS;
} v0_repl_result_t;

typedef struct {
    UPK_V0_REPL_LISTING_T_FIELDS;
} v0_repl_listing_t;

typedef struct {
    UPK_V0_SVCINFO_T_FIELDS;
} v0_svcinfo_t;

typedef struct {
    UPK_V0_REPL_SVCINFO_T_FIELDS;
} v0_repl_svcinfo_t;

typedef struct {
    UPK_V0_REPL_ACK_T_FIELDS;
} v0_repl_ack_t;

typedef struct {
    UPK_V0_REPL_ERROR_T_FIELDS;
} v0_repl_error_t;

typedef struct {
    UPK_V0_PUBLICATION_T_FIELDS;
} v0_pub_publication_t;

typedef struct {
    UPK_V0_CANCELATION_T_FIELDS;
} v0_pub_cancelation_t;


/* *******************************************************************************************************************
 * function prototypes for serializing, deserializing, creating, and otherwise manipulating packets.
 * ****************************************************************************************************************** */
extern upk_pkt_buf_t   *v0_serialize_payload(upk_packet_t * pkt);   /* , upk_pkttype_t pkttype, size_t size); */
extern void            *v0_deserialize_payload(upk_pkt_buf_t * UPK_BUF, upk_pkttype_t pkttype);

/* *******************************************************************************************************************
 * convenience functions for requests, when its easier than building the structs yourself
 * ****************************************************************************************************************** */
extern upk_packet_t    *v0_create_req_seq_start(upk_msgtype_t msg_seq_type, uint32_t msg_seq_count);
extern upk_packet_t    *v0_create_req_seq_end(bool commit);

extern upk_packet_t    *v0_create_req_action(char *svc_id, char *action);
extern upk_packet_t    *v0_create_req_signal(char *svc_id, upk_signal_t signal, bool signal_sid, bool signal_pgrp);
extern upk_packet_t    *v0_create_req_list(void);
extern upk_packet_t    *v0_create_req_status(char *svc_id);
extern upk_packet_t    *v0_create_req_subscribe(char *svc_id, bool all_svcs);
extern upk_packet_t    *v0_create_req_unsubscribe(char *svc_id, bool all_svcs);
extern upk_packet_t    *v0_create_req_disconnect(void);


/* *******************************************************************************************************************
 * convenience functions for replies, when its easier than building the structs yourself
 * ****************************************************************************************************************** */
extern upk_packet_t    *v0_create_repl_seq_start(upk_msgtype_t msg_seq_type, uint32_t msg_seq_count);
extern upk_packet_t    *v0_create_repl_seq_end(bool commit);

extern upk_packet_t    *v0_create_repl_result(char *msg, bool successful);
extern upk_packet_t    *v0_create_repl_listing(char *svc_id);
extern upk_packet_t    *v0_create_repl_svcinfo(char *svc_id, v0_svcinfo_t * svcinfo);
extern upk_packet_t    *v0_create_repl_ack(void);
extern upk_packet_t    *v0_create_repl_error(char *svc_id, upk_errno_t uerrno, char *errmsg, upk_errlevel_t errlvl);


/* *******************************************************************************************************************
 * convenience functions for pubmsg's, because, why not...
 * ****************************************************************************************************************** */
extern upk_packet_t    *v0_create_pub_publication(void);
extern upk_packet_t    *v0_create_pub_cancelation(void);

/* *******************************************************************************************************************
 * housekeeping
 * ****************************************************************************************************************** */
extern void v0_free_payload(upk_packet_t * pkt);

#endif
