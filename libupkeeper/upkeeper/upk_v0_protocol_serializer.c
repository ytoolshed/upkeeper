#include "v0_protocol.h"
#include "error.h"
#include <assert.h> /* FIXME: this should not be an assert, but should use internal error handling to report invalid packet */

#undef UPK_MSG_IDENTIFIER
#define UPK_MSG_IDENTIFIER msgtype
extern upk_pkttype_t    upk_pkt_proto_limit[];
extern upk_req_msgtype_t upk_req_proto_limit[];
extern upk_repl_msgtype_t upk_repl_proto_limit[];
extern upk_pub_msgtype_t upk_pub_proto_limit[];

typedef void           *(*deserialize_payload_t) (upk_pkt_buf_t *);
typedef upk_pkt_buf_t  *(*serialize_payload_t) (void *, size_t);

/* *******************************************************************************************************************
 * pkttype switches for serialization/deserialization
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *serialize_req_payload(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serialize_repl_payload(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serialize_pub_payload(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);

static inline void            *deserialize_req_payload(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserialize_repl_payload(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserialize_pub_payload(upk_pkt_buf_t * UPK_BUF);

/* *******************************************************************************************************************
 * deserialization functions
 * ****************************************************************************************************************** */
static inline void            *deserial_req_seq_start(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserial_req_seq_end(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserial_req_action(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserial_req_signal(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserial_req_list(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserial_req_status(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserial_req_subscribe(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserial_req_unsub(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserial_req_disconnect(upk_pkt_buf_t * UPK_BUF);

static inline void            *deserial_repl_seq_start(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserial_repl_seq_end(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserial_repl_result(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserial_repl_listing(upk_pkt_buf_t * UPK_BUF);
static inline v0_svcinfo_t    *deserial_svcinfo_data(upk_pkt_buf_t * UPK_BUF); /* A helper function for deserial_repl_svcinfo */
static inline void            *deserial_repl_svcinfo(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserial_repl_ack(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserial_repl_err(upk_pkt_buf_t * UPK_BUF);

static inline void            *deserial_pub_pub(upk_pkt_buf_t * UPK_BUF);
static inline void            *deserial_pub_cancel(upk_pkt_buf_t * UPK_BUF);

/* *******************************************************************************************************************
 * serialization functions
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *serial_req_seq_start(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_req_seq_end(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_req_action(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_req_signal(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_req_list(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_req_status(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_req_subscribe(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_req_unsub(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_req_disconnect(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);

static inline upk_pkt_buf_t   *serial_repl_seq_start(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_repl_seq_end(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_repl_result(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_repl_listing(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_svcinfo_data(v0_svcinfo_t * data, size_t UPK_DATA_LEN);  /* A helper function for
                                                                                         * serial_repl_svcinfo */
static inline upk_pkt_buf_t   *serial_repl_svcinfo(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_repl_ack(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_repl_err(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);

static inline upk_pkt_buf_t   *serial_pub_pub(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
static inline upk_pkt_buf_t   *serial_pub_cancel(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);



/* *******************************************************************************************************************
 * dispatch tables for static inline function selection; avoids both the wasted time (assuming compiler optimization doesn't
 * do this for you), and, more importantly, confusing code consiting of massive switch/case sections.
 * ****************************************************************************************************************** */
deserialize_payload_t   deserial_pkt_dispatch[] = {
    [PKT_REQUEST] = deserialize_req_payload,
    [PKT_REPLY] = deserialize_repl_payload,
    [PKT_PUBMSG] = deserialize_pub_payload,
};

deserialize_payload_t   deserial_req_dispatch[] = {
    [REQ_SEQ_START] = deserial_req_seq_start,
    [REQ_SEQ_END] = deserial_req_seq_end,
    [REQ_ACTION] = deserial_req_action,
    [REQ_SIGNAL] = deserial_req_signal,
    [REQ_LIST] = deserial_req_list,
    [REQ_STATUS] = deserial_req_status,
    [REQ_SUBSCRIBE] = deserial_req_subscribe,
    [REQ_UNSUBSCRIBE] = deserial_req_unsub,
    [REQ_DISCONNECT] = deserial_req_disconnect,
};

deserialize_payload_t   deserial_repl_dispatch[] = {
    [REPL_SEQ_START] = deserial_repl_seq_start,
    [REPL_SEQ_END] = deserial_repl_seq_end,
    [REPL_RESULT] = deserial_repl_result,
    [REPL_LISTING] = deserial_repl_listing,
    [REPL_SVCINFO] = deserial_repl_svcinfo,
    [REPL_ACK] = deserial_repl_ack,
    [REPL_ERROR] = deserial_repl_err,
};

deserialize_payload_t   deserial_pub_dispatch[] = {
    [PUB_PUBLICATION] = deserial_pub_pub,
    [PUB_CANCELATION] = deserial_pub_cancel,
};

serialize_payload_t     serial_pkt_dispatch[] = {
    [PKT_REQUEST] = serialize_req_payload,
    [PKT_REPLY] = serialize_repl_payload,
    [PKT_PUBMSG] = serialize_pub_payload,
};

serialize_payload_t     serial_req_dispatch[] = {
    [REQ_SEQ_START] = serial_req_seq_start,
    [REQ_SEQ_END] = serial_req_seq_end,
    [REQ_ACTION] = serial_req_action,
    [REQ_SIGNAL] = serial_req_signal,
    [REQ_LIST] = serial_req_list,
    [REQ_STATUS] = serial_req_status,
    [REQ_SUBSCRIBE] = serial_req_subscribe,
    [REQ_UNSUBSCRIBE] = serial_req_unsub,
    [REQ_DISCONNECT] = serial_req_disconnect,
};

serialize_payload_t     serial_repl_dispatch[] = {
    [REPL_SEQ_START] = serial_repl_seq_start,
    [REPL_SEQ_END] = serial_repl_seq_end,
    [REPL_RESULT] = serial_repl_result,
    [REPL_LISTING] = serial_repl_listing,
    [REPL_SVCINFO] = serial_repl_svcinfo,
    [REPL_ACK] = serial_repl_ack,
    [REPL_ERROR] = serial_repl_err,
};

serialize_payload_t     serial_pub_dispatch[] = {
    [PUB_PUBLICATION] = serial_pub_pub,
    [PUB_CANCELATION] = serial_pub_cancel,
};

/* *******************************************************************************************************************
 * payload switches
 * ****************************************************************************************************************** */

/* *******************************************************************************************************************
 * deserialize
 * ****************************************************************************************************************** */
void                   *
v0_deserialize_payload(upk_pkt_buf_t * UPK_BUF, upk_pkttype_t pkttype)
{
    return (deserial_pkt_dispatch[pkttype]) (UPK_BUF);
}

/* *******************************************************************************************************************
 * serialize
 * ****************************************************************************************************************** */
upk_pkt_buf_t          *
v0_serialize_payload(upk_packet_t * pkt)
{
    return (serial_pkt_dispatch[pkt->pkttype]) (pkt->payload, pkt->payload_len);
}


/* *******************************************************************************************************************
 * *********************************************************************************************************************
 * ** request-type switches
 * *********************************************************************************************************************
 * ****************************************************************************************************************** */

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserialize_req_payload(upk_pkt_buf_t * UPK_BUF)
{
    uint32_t                enum_buf;
    upk_req_msgtype_t       msgtype;

    memcpy(&enum_buf, UPK_BUF, sizeof(enum_buf));
    msgtype = (upk_req_msgtype_t) ntohl(enum_buf);
    assert(msgtype < sizeof(deserial_req_dispatch));

    return (deserial_req_dispatch[msgtype]) (UPK_BUF);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserialize_repl_payload(upk_pkt_buf_t * UPK_BUF)
{
    uint32_t                enum_buf;
    upk_repl_msgtype_t      msgtype;

    memcpy(&enum_buf, UPK_BUF, sizeof(enum_buf));
    msgtype = (upk_repl_msgtype_t) ntohl(enum_buf);
    assert(msgtype < sizeof(deserial_repl_dispatch));

    return (deserial_repl_dispatch[msgtype]) (UPK_BUF);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserialize_pub_payload(upk_pkt_buf_t * UPK_BUF)
{
    uint32_t                enum_buf;
    upk_pub_msgtype_t       msgtype;

    memcpy(&enum_buf, UPK_BUF, sizeof(enum_buf));
    msgtype = (upk_pub_msgtype_t) ntohl(enum_buf);
    assert(msgtype < sizeof(deserial_pub_dispatch));

    return (deserial_pub_dispatch[msgtype]) (UPK_BUF);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serialize_req_payload(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    return (serial_req_dispatch[((upk_req_preamble_t *) UPK_DATA_PTR)->msgtype]) (UPK_DATA_PTR, UPK_DATA_LEN);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serialize_repl_payload(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    return (serial_repl_dispatch[((upk_repl_preamble_t *) UPK_DATA_PTR)->msgtype]) (UPK_DATA_PTR, UPK_DATA_LEN);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serialize_pub_payload(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    return (serial_pub_dispatch[((upk_pub_pubmsg_t *) UPK_DATA_PTR)->msgtype]) (UPK_DATA_PTR, UPK_DATA_LEN);
}

/* *******************************************************************************************************************
 * *********************************************************************************************************************
 * ** Deserialization
 * *********************************************************************************************************************
 * ****************************************************************************************************************** */

/* *******************************************************************************************************************
 * REQUESTS
 * ****************************************************************************************************************** */
#undef UPK_MSG_IDENTIFIER_TYPEDEF
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_req_msgtype_t

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_req_seq_start(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_req_seq_start_t);

    UPK_FETCH_ENUM(upk_req_msgtype_t, msg_seq_type);
    UPK_FETCH_UINT32(msg_seq_count);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_req_seq_end(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_req_seq_end_t);

    UPK_FETCH_BOOL(commit);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_req_action(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_action_req_t);

    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);
    UPK_FETCH_UINT32(action_len);
    UPK_FETCH_STRING(action);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_req_signal(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_signal_req_t);

    UPK_FETCH_ENUM(upk_signal_t, signal);
    UPK_FETCH_BOOL(signal_sid);
    UPK_FETCH_BOOL(signal_pgrp);
    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_req_list(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_list_req_t);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_req_status(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_status_req_t);

    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_req_subscribe(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_subscr_req_t);

    UPK_FETCH_BOOL(all_svcs);
    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_req_unsub(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_unsubs_req_t);

    UPK_FETCH_BOOL(all_svcs);
    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_req_disconnect(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_discon_req_t);

    return UPK_DATA;
}


/* *******************************************************************************************************************
 * REPLIES
 * ****************************************************************************************************************** */
#undef UPK_MSG_IDENTIFIER_TYPEDEF
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_repl_msgtype_t

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_repl_seq_start(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_repl_seq_start_t);

    UPK_FETCH_ENUM(upk_repl_msgtype_t, msg_seq_type);
    UPK_FETCH_UINT32(msg_seq_count);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_repl_seq_end(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_repl_seq_end_t);

    UPK_FETCH_BOOL(commit);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_repl_result(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_result_repl_t);

    UPK_FETCH_BOOL(successful);
    UPK_FETCH_UINT32(msg_len);
    UPK_FETCH_STRING(msg);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_repl_listing(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_listing_repl_t);

    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */

static inline v0_svcinfo_t    *
deserial_svcinfo_data(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE(v0_svcinfo_t);
    UPK_DATA = calloc(1,sizeof(*UPK_DATA));

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

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_repl_svcinfo(upk_pkt_buf_t * UPK_BUF)
{
    upk_pkt_buf_t          *data;
    v0_svcinfo_t           *svcinfo_buf;

    UPK_INIT_DESERIALIZE_MSG(v0_svcinfo_repl_t);

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
 * ****************************************************************************************************************** */
static inline void            *
deserial_repl_ack(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(v0_ack_repl_t);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_repl_err(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(upk_error_repl_t);

    UPK_FETCH_ENUM(upk_errlevel_t, errlevel);
    UPK_FETCH_UINT32(msg_len);
    UPK_FETCH_STRING(msg);
    UPK_FETCH_UINT32(svc_id_len);
    UPK_FETCH_STRING(svc_id);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * PUBMSG's
 * ****************************************************************************************************************** */
#undef UPK_MSG_IDENTIFIER_TYPEDEF
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_pub_msgtype_t

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_pub_pub(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(upk_pub_pubmsg_t);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void            *
deserial_pub_cancel(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(upk_cancel_pubmsg_t);

    return UPK_DATA;
}

/* *******************************************************************************************************************
 * *********************************************************************************************************************
 * ** serialization
 * *********************************************************************************************************************
 * ****************************************************************************************************************** */

/* *******************************************************************************************************************
 * requests
 * ****************************************************************************************************************** */
#undef UPK_MSG_IDENTIFIER_TYPEDEF
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_req_msgtype_t

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_req_seq_start(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(upk_req_seq_start_t);

    UPK_PUT_ENUM(msg_seq_type);
    UPK_PUT_UINT32(msg_seq_count);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_req_seq_end(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(upk_req_seq_end_t);

    UPK_PUT_BOOL(commit);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_req_action(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_action_req_t);

    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);
    UPK_PUT_UINT32(action_len);
    UPK_PUT_STRING(action);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_req_signal(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_signal_req_t);

    UPK_PUT_ENUM(signal);
    UPK_PUT_BOOL(signal_sid);
    UPK_PUT_BOOL(signal_pgrp);
    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_req_list(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_list_req_t);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_req_status(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_status_req_t);

    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_req_subscribe(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_subscr_req_t);

    UPK_PUT_BOOL(all_svcs);
    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_req_unsub(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_unsubs_req_t);

    UPK_PUT_BOOL(all_svcs);
    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_req_disconnect(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_discon_req_t);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * replies
 * ****************************************************************************************************************** */
#undef UPK_MSG_IDENTIFIER_TYPEDEF
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_repl_msgtype_t

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_repl_seq_start(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_repl_seq_start_t);

    UPK_PUT_ENUM(msg_seq_type);
    UPK_PUT_UINT32(msg_seq_count);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_repl_seq_end(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_repl_seq_end_t);

    UPK_PUT_BOOL(commit);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_repl_result(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_result_repl_t);

    UPK_PUT_BOOL(successful);
    UPK_PUT_UINT32(msg_len);
    UPK_PUT_STRING(msg);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_repl_listing(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_listing_repl_t);

    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
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

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_repl_svcinfo(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    upk_pkt_buf_t          *data;
    UPK_INIT_SERIALIZE_MSG(v0_svcinfo_repl_t);

    UPK_PUT_UINT32(svcinfo_len);
    
    data = serial_svcinfo_data(&UPK_DATA->svcinfo, UPK_DATA->svcinfo_len);
    UPK_PUT_DATA_FROM_BUF(data, svcinfo_len);
    free(data);

    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_repl_ack(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(v0_ack_repl_t);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_repl_err(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(upk_error_repl_t);

    UPK_PUT_ENUM(errlevel);
    UPK_PUT_UINT32(msg_len);
    UPK_PUT_STRING(msg);
    UPK_PUT_UINT32(svc_id_len);
    UPK_PUT_STRING(svc_id);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * PUBMSG's
 * ****************************************************************************************************************** */
#undef UPK_MSG_IDENTIFIER_TYPEDEF
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_pub_msgtype_t

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_pub_pub(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(upk_pub_pubmsg_t);

    return UPK_BUF;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline upk_pkt_buf_t   *
serial_pub_cancel(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(upk_cancel_pubmsg_t);

    return UPK_BUF;
}
