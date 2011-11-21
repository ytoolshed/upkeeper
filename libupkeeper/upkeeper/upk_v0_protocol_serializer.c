
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

#include "upk_v0_protocol.h"
#include "upk_error.h"
#include <assert.h>                                        /* FIXME: this should not be an assert, but should 
                                                              use internal error handling to report invalid
                                                              packet */

#undef UPK_MSG_IDENTIFIER
#define UPK_MSG_IDENTIFIER msgtype
extern upk_pkttype_t    upk_pkt_proto_limit[];
extern upk_msgtype_t    upk_req_proto_limit[];
extern upk_msgtype_t    upk_repl_proto_limit[];
extern upk_msgtype_t    upk_pub_proto_limit[];

typedef void           *(*deserialize_payload_t) (upk_pkt_buf_t *);
typedef upk_pkt_buf_t  *(*serialize_payload_t) (void *, size_t);

/* *******************************************************************************************************************
   pkttype switches for serialization/deserialization
   ****************************************************************************************************************** */

static upk_pkt_buf_t   *serialize_req_payload(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serialize_repl_payload(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serialize_pub_payload(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);

static void            *deserialize_req_payload(upk_pkt_buf_t * UPK_BUF);
static void            *deserialize_repl_payload(upk_pkt_buf_t * UPK_BUF);
static void            *deserialize_pub_payload(upk_pkt_buf_t * UPK_BUF);

/* *******************************************************************************************************************
   deserialization functions
   ****************************************************************************************************************** */
static void            *deserial_req_seq_start(upk_pkt_buf_t * UPK_BUF);
static void            *deserial_req_seq_end(upk_pkt_buf_t * UPK_BUF);
static void            *deserial_req_action(upk_pkt_buf_t * UPK_BUF);
static void            *deserial_req_signal(upk_pkt_buf_t * UPK_BUF);
static void            *deserial_req_list(upk_pkt_buf_t * UPK_BUF);
static void            *deserial_req_status(upk_pkt_buf_t * UPK_BUF);
static void            *deserial_req_subscribe(upk_pkt_buf_t * UPK_BUF);
static void            *deserial_req_unsub(upk_pkt_buf_t * UPK_BUF);
static void            *deserial_req_disconnect(upk_pkt_buf_t * UPK_BUF);

static void            *deserial_repl_seq_start(upk_pkt_buf_t * UPK_BUF);
static void            *deserial_repl_seq_end(upk_pkt_buf_t * UPK_BUF);
static void            *deserial_repl_result(upk_pkt_buf_t * UPK_BUF);
static void            *deserial_repl_listing(upk_pkt_buf_t * UPK_BUF);
static inline v0_svcinfo_t *deserial_svcinfo_data(upk_pkt_buf_t * UPK_BUF); /* A helper function for
                                                                               deserial_repl_svcinfo */
static void            *deserial_repl_svcinfo(upk_pkt_buf_t * UPK_BUF);
static void            *deserial_repl_ack(upk_pkt_buf_t * UPK_BUF);
static void            *deserial_repl_err(upk_pkt_buf_t * UPK_BUF);

static void            *deserial_pub_pub(upk_pkt_buf_t * UPK_BUF);
static void            *deserial_pub_cancel(upk_pkt_buf_t * UPK_BUF);

/* *******************************************************************************************************************
   serialization functions
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *serial_req_seq_start(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serial_req_seq_end(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serial_req_action(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serial_req_signal(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serial_req_list(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serial_req_status(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serial_req_subscribe(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serial_req_unsub(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serial_req_disconnect(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);

static upk_pkt_buf_t   *serial_repl_seq_start(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serial_repl_seq_end(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serial_repl_result(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serial_repl_listing(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t *serial_svcinfo_data(v0_svcinfo_t * data, size_t UPK_DATA_LEN); /* A helper
                                                                                               function for *
                                                                                               serial_repl_svcinfo 
                                                                                             */
static upk_pkt_buf_t   *serial_repl_svcinfo(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serial_repl_ack(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serial_repl_err(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);

static upk_pkt_buf_t   *serial_pub_pub(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static upk_pkt_buf_t   *serial_pub_cancel(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);



/* *******************************************************************************************************************
   dispatch tables for static inline function selection; avoids both the wasted time (assuming compiler optimization
   doesn't do this for you), and, more importantly, confusing code consiting of massive switch/case sections.
   ****************************************************************************************************************** */
static deserialize_payload_t deserial_pkt_dispatch[] = {
    [PKT_REQUEST] = deserialize_req_payload,
    [PKT_REPLY] = deserialize_repl_payload,
    [PKT_PUBMSG] = deserialize_pub_payload,
};

static deserialize_payload_t deserial_req_dispatch[] = {
    [UPK_REQ_PREAMBLE - UPK_REQ_ORIGIN] = upk_deserialize_req_preamble,
    [UPK_REQ_SEQ_START - UPK_REQ_ORIGIN] = deserial_req_seq_start,
    [UPK_REQ_SEQ_END - UPK_REQ_ORIGIN] = deserial_req_seq_end,
    [UPK_REQ_ACTION - UPK_REQ_ORIGIN] = deserial_req_action,
    [UPK_REQ_SIGNAL - UPK_REQ_ORIGIN] = deserial_req_signal,
    [UPK_REQ_LIST - UPK_REQ_ORIGIN] = deserial_req_list,
    [UPK_REQ_STATUS - UPK_REQ_ORIGIN] = deserial_req_status,
    [UPK_REQ_SUBSCRIBE - UPK_REQ_ORIGIN] = deserial_req_subscribe,
    [UPK_REQ_UNSUBSCRIBE - UPK_REQ_ORIGIN] = deserial_req_unsub,
    [UPK_REQ_DISCONNECT - UPK_REQ_ORIGIN] = deserial_req_disconnect,
};

static deserialize_payload_t deserial_repl_dispatch[] = {
    [UPK_REPL_PREAMBLE - UPK_REPL_ORIGIN] = upk_deserialize_repl_preamble,
    [UPK_REPL_SEQ_START - UPK_REPL_ORIGIN] = deserial_repl_seq_start,
    [UPK_REPL_SEQ_END - UPK_REPL_ORIGIN] = deserial_repl_seq_end,
    [UPK_REPL_RESULT - UPK_REPL_ORIGIN] = deserial_repl_result,
    [UPK_REPL_LISTING - UPK_REPL_ORIGIN] = deserial_repl_listing,
    [UPK_REPL_SVCINFO - UPK_REPL_ORIGIN] = deserial_repl_svcinfo,
    [UPK_REPL_ACK - UPK_REPL_ORIGIN] = deserial_repl_ack,
    [UPK_REPL_ERROR - UPK_REPL_ORIGIN] = deserial_repl_err,
};

static deserialize_payload_t deserial_pub_dispatch[] = {
    [UPK_PUB_PUBLICATION - UPK_PUB_ORIGIN] = deserial_pub_pub,
    [UPK_PUB_CANCELATION - UPK_PUB_ORIGIN] = deserial_pub_cancel,
};

static serialize_payload_t serial_pkt_dispatch[] = {
    [PKT_REQUEST] = serialize_req_payload,
    [PKT_REPLY] = serialize_repl_payload,
    [PKT_PUBMSG] = serialize_pub_payload,
};

static serialize_payload_t serial_req_dispatch[] = {
    [UPK_REQ_PREAMBLE - UPK_REQ_ORIGIN] = upk_serialize_req_preamble,
    [UPK_REQ_SEQ_START - UPK_REQ_ORIGIN] = serial_req_seq_start,
    [UPK_REQ_SEQ_END - UPK_REQ_ORIGIN] = serial_req_seq_end,
    [UPK_REQ_ACTION - UPK_REQ_ORIGIN] = serial_req_action,
    [UPK_REQ_SIGNAL - UPK_REQ_ORIGIN] = serial_req_signal,
    [UPK_REQ_LIST - UPK_REQ_ORIGIN] = serial_req_list,
    [UPK_REQ_STATUS - UPK_REQ_ORIGIN] = serial_req_status,
    [UPK_REQ_SUBSCRIBE - UPK_REQ_ORIGIN] = serial_req_subscribe,
    [UPK_REQ_UNSUBSCRIBE - UPK_REQ_ORIGIN] = serial_req_unsub,
    [UPK_REQ_DISCONNECT - UPK_REQ_ORIGIN] = serial_req_disconnect,
};

static serialize_payload_t serial_repl_dispatch[] = {
    [UPK_REPL_PREAMBLE - UPK_REPL_ORIGIN] = upk_serialize_repl_preamble,
    [UPK_REPL_SEQ_START - UPK_REPL_ORIGIN] = serial_repl_seq_start,
    [UPK_REPL_SEQ_END - UPK_REPL_ORIGIN] = serial_repl_seq_end,
    [UPK_REPL_RESULT - UPK_REPL_ORIGIN] = serial_repl_result,
    [UPK_REPL_LISTING - UPK_REPL_ORIGIN] = serial_repl_listing,
    [UPK_REPL_SVCINFO - UPK_REPL_ORIGIN] = serial_repl_svcinfo,
    [UPK_REPL_ACK - UPK_REPL_ORIGIN] = serial_repl_ack,
    [UPK_REPL_ERROR - UPK_REPL_ORIGIN] = serial_repl_err,
};

static serialize_payload_t serial_pub_dispatch[] = {
    [UPK_PUB_PUBLICATION - UPK_PUB_ORIGIN] = serial_pub_pub,
    [UPK_PUB_CANCELATION - UPK_PUB_ORIGIN] = serial_pub_cancel,
};

/* *******************************************************************************************************************
   payload switches
   ****************************************************************************************************************** */

/* *******************************************************************************************************************
   deserialize
   ****************************************************************************************************************** */
void                   *
v0_deserialize_payload(upk_pkt_buf_t * UPK_BUF, upk_pkttype_t pkttype)
{
    return (deserial_pkt_dispatch[pkttype]) (UPK_BUF);
}

/* *******************************************************************************************************************
   serialize
   ****************************************************************************************************************** */
upk_pkt_buf_t          *
v0_serialize_payload(upk_packet_t * pkt)
{
    return (serial_pkt_dispatch[pkt->pkttype]) (pkt->payload, pkt->payload_len);
}


/* *******************************************************************************************************************
   *********************************************************************************************************************
   ** request-type switches
   *********************************************************************************************************************
   ****************************************************************************************************************** */

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserialize_req_payload(upk_pkt_buf_t * UPK_BUF)
{
    uint32_t                enum_buf;
    upk_msgtype_t           msgtype;

    memcpy(&enum_buf, UPK_BUF, sizeof(enum_buf));
    msgtype = (upk_msgtype_t) ntohl(enum_buf);
    assert((msgtype - UPK_REQ_ORIGIN) < (sizeof(deserial_req_dispatch) / sizeof(*deserial_req_dispatch)));

    return (deserial_req_dispatch[msgtype - UPK_REQ_ORIGIN]) (UPK_BUF);
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserialize_repl_payload(upk_pkt_buf_t * UPK_BUF)
{
    uint32_t                enum_buf;
    upk_msgtype_t           msgtype;

    memcpy(&enum_buf, UPK_BUF, sizeof(enum_buf));
    msgtype = (upk_msgtype_t) ntohl(enum_buf);
    assert((msgtype - UPK_REPL_ORIGIN) < (sizeof(deserial_repl_dispatch) / sizeof(*deserial_repl_dispatch)));

    return (deserial_repl_dispatch[msgtype - UPK_REPL_ORIGIN]) (UPK_BUF);
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserialize_pub_payload(upk_pkt_buf_t * UPK_BUF)
{
    uint32_t                enum_buf;
    upk_msgtype_t           msgtype;

    memcpy(&enum_buf, UPK_BUF, sizeof(enum_buf));
    msgtype = (upk_msgtype_t) ntohl(enum_buf);
    assert((msgtype - UPK_PUB_ORIGIN) < (sizeof(deserial_pub_dispatch) / sizeof(*deserial_pub_dispatch)));

    return (deserial_pub_dispatch[msgtype - UPK_PUB_ORIGIN]) (UPK_BUF);
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serialize_req_payload(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    upk_generic_msg_t      *dummy = UPK_DATA_PTR;

    return (serial_req_dispatch[dummy->msgtype - UPK_REQ_ORIGIN]) (UPK_DATA_PTR, UPK_DATA_LEN);
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serialize_repl_payload(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    upk_generic_msg_t      *dummy = UPK_DATA_PTR;

    return (serial_repl_dispatch[dummy->msgtype - UPK_REPL_ORIGIN]) (UPK_DATA_PTR, UPK_DATA_LEN);
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serialize_pub_payload(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    upk_generic_msg_t      *dummy = UPK_DATA_PTR;

    return (serial_pub_dispatch[dummy->msgtype - UPK_PUB_ORIGIN]) (UPK_DATA_PTR, UPK_DATA_LEN);
}

/* *******************************************************************************************************************
   *********************************************************************************************************************
   ** Deserialization
   *********************************************************************************************************************
   ****************************************************************************************************************** */

/* *******************************************************************************************************************
   REQUESTS
   ****************************************************************************************************************** */
#undef UPK_MSG_IDENTIFIER_TYPEDEF
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_msgtype_t

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_req_seq_start(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_req_seq_start_t);

    UPK_FETCH_ENUM(upk_msgtype_t, msg_seq_type);
    UPK_FETCH_UINT32(msg_seq_count);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_req_seq_end(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_req_seq_end_t);

    UPK_FETCH_BOOL(commit);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_req_action(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_req_action_t);

    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);
    UPK_FETCH_UINT32(action_len);
    UPK_FETCH_STRING(action);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_req_signal(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_req_signal_t);

    UPK_FETCH_ENUM(upk_signal_t, signal);
    UPK_FETCH_BOOL(signal_sid);
    UPK_FETCH_BOOL(signal_pgrp);
    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_req_list(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_req_list_t);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_req_status(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_req_status_t);

    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);
    UPK_FETCH_UINT32(restart_window_seconds);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_req_subscribe(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_req_subscribe_t);

    UPK_FETCH_BOOL(all_svcs);
    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_req_unsub(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_req_unsubscribe_t);

    UPK_FETCH_BOOL(all_svcs);
    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_req_disconnect(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_req_disconnect_t);

    return UPK_DATA;
}


/* *******************************************************************************************************************
   REPLIES
   ****************************************************************************************************************** */
#undef UPK_MSG_IDENTIFIER_TYPEDEF
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_msgtype_t

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_repl_seq_start(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_repl_seq_start_t);

    UPK_FETCH_ENUM(upk_msgtype_t, msg_seq_type);
    UPK_FETCH_UINT32(msg_seq_count);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_repl_seq_end(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_repl_seq_end_t);

    UPK_FETCH_BOOL(commit);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_repl_result(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_repl_result_t);

    UPK_FETCH_BOOL(successful);
    UPK_FETCH_UINT32(msg_len);
    UPK_FETCH_STRING(msg);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_repl_listing(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_repl_listing_t);

    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */

static inline v0_svcinfo_t *
deserial_svcinfo_data(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE(v0_svcinfo_t);
    UPK_DATA = calloc(1, sizeof(*UPK_DATA));

    UPK_FETCH_UINT32(last_action_time);
    UPK_FETCH_UINT32(last_action_status);
    UPK_FETCH_UINT32(last_action_name_len);
    UPK_FETCH_ARRAY(last_action_name, UPK_DATA->last_action_name_len);
    UPK_FETCH_UINT32(last_signal_time);
    UPK_FETCH_UINT32(last_signal_status);
    UPK_FETCH_ENUM(upk_signal_t, last_signal_name);
    UPK_FETCH_UINT32(buddy_pid);
    UPK_FETCH_UINT32(proc_pid);
    UPK_FETCH_ENUM(upk_state_t, current_state);
    UPK_FETCH_ENUM(upk_state_t, prior_state);
    UPK_FETCH_UINT32(n_recorded_restarts);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_repl_svcinfo(upk_pkt_buf_t * UPK_BUF)
{
    upk_pkt_buf_t          *data;
    v0_svcinfo_t           *svcinfo_buf;

    UPK_INIT_DESERIALIZE_MSG(v0_repl_svcinfo_t);

    UPK_FETCH_UINT32(svcinfo_len);
    UPK_FETCH_DATA_TO_BUF(data, svcinfo_len);

    svcinfo_buf = deserial_svcinfo_data(data);
    UPK_DATA->svcinfo = *svcinfo_buf;
    free(svcinfo_buf);
    free(data);

    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_repl_ack(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_repl_ack_t);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_repl_err(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_repl_error_t);

    UPK_FETCH_ENUM(upk_errlevel_t, errlevel);
    UPK_FETCH_ENUM(upk_errno_t, uerrno);
    UPK_FETCH_UINT32(msg_len);
    UPK_FETCH_STRING(msg);
    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   PUBMSG's
   ****************************************************************************************************************** */
#undef UPK_MSG_IDENTIFIER_TYPEDEF
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_msgtype_t

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_pub_pub(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_pub_publication_t);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void            *
deserial_pub_cancel(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_pub_cancelation_t);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   *********************************************************************************************************************
   ** serialization
   *********************************************************************************************************************
   ****************************************************************************************************************** */

/* *******************************************************************************************************************
   requests
   ****************************************************************************************************************** */
#undef UPK_MSG_IDENTIFIER_TYPEDEF
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_msgtype_t

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_req_seq_start(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(upk_req_seq_start_t);

    UPK_PUT_ENUM(msg_seq_type);
    UPK_PUT_UINT32(msg_seq_count);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_req_seq_end(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(upk_req_seq_end_t);

    UPK_PUT_BOOL(commit);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_req_action(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_req_action_t);

    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);
    UPK_PUT_UINT32(action_len);
    UPK_PUT_STRING(action);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_req_signal(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_req_signal_t);

    UPK_PUT_ENUM(signal);
    UPK_PUT_BOOL(signal_sid);
    UPK_PUT_BOOL(signal_pgrp);
    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_req_list(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_req_list_t);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_req_status(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_req_status_t);

    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);
    UPK_PUT_UINT32(restart_window_seconds);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_req_subscribe(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_req_subscribe_t);

    UPK_PUT_BOOL(all_svcs);
    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_req_unsub(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_req_unsubscribe_t);

    UPK_PUT_BOOL(all_svcs);
    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_req_disconnect(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_req_disconnect_t);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   replies
   ****************************************************************************************************************** */
#undef UPK_MSG_IDENTIFIER_TYPEDEF
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_msgtype_t

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_repl_seq_start(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_repl_seq_start_t);

    UPK_PUT_ENUM(msg_seq_type);
    UPK_PUT_UINT32(msg_seq_count);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_repl_seq_end(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_repl_seq_end_t);

    UPK_PUT_BOOL(commit);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_repl_result(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_repl_result_t);

    UPK_PUT_BOOL(successful);
    UPK_PUT_UINT32(msg_len);
    UPK_PUT_STRING(msg);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_repl_listing(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_repl_listing_t);

    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static inline upk_pkt_buf_t *
serial_svcinfo_data(v0_svcinfo_t * UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_BUF(v0_svcinfo_t, UPK_DATA_LEN);

    UPK_PUT_UINT32(last_action_time);
    UPK_PUT_UINT32(last_action_status);
    UPK_PUT_UINT32(last_action_name_len);
    UPK_PUT_ARRAY(last_action_name, UPK_DATA->last_action_name_len);
    UPK_PUT_UINT32(last_signal_time);
    UPK_PUT_UINT32(last_signal_status);
    UPK_PUT_ENUM(last_signal_name);
    UPK_PUT_UINT32(buddy_pid);
    UPK_PUT_UINT32(proc_pid);
    UPK_PUT_ENUM(current_state);
    UPK_PUT_ENUM(prior_state);
    UPK_PUT_UINT32(n_recorded_restarts);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_repl_svcinfo(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    upk_pkt_buf_t          *data;

    UPK_INIT_SERIALIZE_MSG(v0_repl_svcinfo_t);

    UPK_PUT_UINT32(svcinfo_len);

    data = serial_svcinfo_data(&UPK_DATA->svcinfo, UPK_DATA->svcinfo_len);
    UPK_PUT_DATA_FROM_BUF(data, svcinfo_len);
    free(data);

    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_repl_ack(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_repl_ack_t);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_repl_err(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_repl_error_t);

    UPK_PUT_ENUM(errlevel);
    UPK_PUT_ENUM(uerrno);
    UPK_PUT_UINT32(msg_len);
    UPK_PUT_STRING(msg);
    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   PUBMSG's
   ****************************************************************************************************************** */
#undef UPK_MSG_IDENTIFIER_TYPEDEF
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_msgtype_t

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_pub_pub(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_pub_publication_t);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static upk_pkt_buf_t   *
serial_pub_cancel(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_pub_cancelation_t);

    return UPK_BUF;
}
