#include "v0_protocol.h"
#define PROTOCOL_VERSION 0
#define CALC_PTR_LEN(M) (payload->M - sizeof(size_t))      /* the resulting payload size will have a size_t subtracted
                                                            * from it (the pointer itself) and instead have the length
                                                            * of data that was pointed to in its place; its easier to
                                                            * calculate that here */

#define CALC_ENUM_LEN(M) (sizeof(uint32_t) - sizeof(payload->M))
#define CALC_BOOL_LEN(M) (sizeof(uint8_t) - sizeof(payload-M))

#define COPY_STRING_MEMB(MEMB, LEN) payload->LEN = strnlen(MEMB, UPK_MAX_STRING_LEN); \
                               payload->MEMB = calloc(1, payload->LEN + 1); \
                               strncpy(payload->MEMB, MEMB, payload->LEN + 1)


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_req_seq_start(upk_req_msgtype_t msg_seq_type, uint32_t msg_seq_count)
{
    v0_req_seq_start_t     *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_SEQ_START;
    payload->msg_seq_type = msg_seq_type;
    payload->msg_seq_count = msg_seq_count;

    payload_len =
        sizeof(*payload) + CALC_ENUM_LEN(msgtype) + CALC_ENUM_LEN(msg_seq_type) + CALC_ENUM_LEN(msg_seq_count);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_req_seq_end(bool commit)
{
    v0_req_seq_end_t       *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_SEQ_END;
    payload->commit = commit;

    payload_len = sizeof(*payload) + CALC_ENUM_LEN(msgtype) + CALC_BOOL_LEN(commit);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_action_req(char *svc_id, char *action)
{
    v0_action_req_t        *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_ACTION;
    COPY_STRING_MEMB(svc_id, svc_id_len);
    COPY_STRING_MEMB(action, action_len);

    payload_len = sizeof(*payload) + CALC_ENUM_LEN(msgtype) + CALC_PTR_LEN(svc_id_len) + CALC_PTR_LEN(action_len);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_signal_req(char *svc_id, uint8_t signal, bool signal_sid, bool signal_pgrp)
{
    v0_signal_req_t        *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_SIGNAL;
    payload->signal = signal;
    payload->signal_sid = signal_sid;
    payload->signal_pgrp = signal_pgrp;
    COPY_STRING_MEMB(svc_id, svc_id_len);

    payload_len =
        sizeof(*payload) + CALC_ENUM_LEN(msgtype) + CALC_BOOL_LEN(signal_sid) + CALC_BOOL_LEN(signal_pgrp) +
        CALC_PTR_LEN(svc_id_len);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_list_req(void)
{
    v0_list_req_t          *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_LIST;

    payload_len = sizeof(*payload) + CALC_ENUM_LEN(msgtype);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_status_req(char *svc_id)
{
    v0_status_req_t        *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_STATUS;
    COPY_STRING_MEMB(svc_id, svc_id_len);

    payload_len = sizeof(*payload) + CALC_ENUM_LEN(msgtype) + CALC_PTR_LEN(svc_id_len);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_subscr_req(char *svc_id, bool all_svcs)
{
    v0_subscribe_req_t     *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_SUBSCRIBE;
    COPY_STRING_MEMB(svc_id, svc_id_len);

    payload_len = sizeof(*payload) + CALC_ENUM_LEN(msgtype) + CALC_PTR_LEN(svc_id_len);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}



/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_unsubs_req(char *svc_id, bool all_svcs)
{
    v0_unsubscribe_req_t   *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_UNSUBSCRIBE;
    payload->all_svcs = all_svcs;
    COPY_STRING_MEMB(svc_id, svc_id_len);

    payload_len = sizeof(*payload) + CALC_ENUM_LEN(msgtype) + CALC_BOOL_LEN(all_svcs) + CALC_PTR_LEN(svc_id_len);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_disconnect_req(void)
{
    v0_disconnect_req_t    *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_DISCONNECT;

    payload_len = sizeof(*payload) + CALC_ENUM_LEN(msgtype);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_repl_seq_start(upk_repl_msgtype_t msg_seq_type, uint32_t msg_seq_count)
{
    v0_repl_seq_start_t    *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REPL_SEQ_START;
    payload->msg_seq_type = msg_seq_type;
    payload->msg_seq_count = msg_seq_count;

    payload_len =
        sizeof(*payload) + CALC_ENUM_LEN(msgtype) + CALC_ENUM_LEN(msg_seq_type) + CALC_ENUM_LEN(msg_seq_count);
    return upk_create_pkt(payload, payload_len, PKT_REPLY, PROTOCOL_VERSION);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_repl_seq_end(bool commit)
{
    v0_repl_seq_end_t      *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REPL_SEQ_END;
    payload->commit = commit;

    payload_len = sizeof(*payload) + CALC_ENUM_LEN(msgtype) + CALC_BOOL_LEN(commit);
    return upk_create_pkt(payload, payload_len, PKT_REPLY, PROTOCOL_VERSION);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_result_repl(char *msg, bool successful)
{
    v0_result_repl_t       *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REPL_RESULT;
    payload->successful = successful;
    COPY_STRING_MEMB(msg, msg_len);

    payload_len = sizeof(*payload) + CALC_ENUM_LEN(msgtype) + CALC_BOOL_LEN(successful) + CALC_PTR_LEN(msg_len);
    return upk_create_pkt(payload, payload_len, PKT_REPLY, PROTOCOL_VERSION);
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_listing_repl(char *svc_id)
{
    v0_listing_repl_t      *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REPL_LISTING;
    COPY_STRING_MEMB(svc_id, svc_id_len);

    payload_len = sizeof(*payload) + CALC_ENUM_LEN(msgtype) + CALC_PTR_LEN(svc_id_len);
    return upk_create_pkt(payload, payload_len, PKT_REPLY, PROTOCOL_VERSION);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static                  uint32_t
get_svcinfo_len(v0_svcinfo_t svcinfo)
{
    uint32_t                length = 0;

    length += sizeof(svcinfo.last_action_time);
    length += sizeof(svcinfo.last_action_status);
    length += sizeof(svcinfo.last_action_name_len);
    length += svcinfo.last_action_name_len;
    length += sizeof(svcinfo.last_signal_time);
    length += sizeof(svcinfo.last_signal_status);
    length += sizeof(uint32_t);                            /* upk_signal_name_t last_signal_name; */
    length += sizeof(svcinfo.buddy_pid);
    length += sizeof(svcinfo.proc_pid);
    length += sizeof(uint32_t);                            /* upk_state_t current_state; */
    length += sizeof(uint32_t);                            /* upk_state_t prior_state); */

    return length;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_svcinfo_repl(char *svc_id, v0_svcinfo_t * svcinfo)
{
    v0_svcinfo_repl_t      *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REPL_SVCINFO;
    payload->svcinfo_len = get_svcinfo_len(payload->svcinfo);
    memcpy(&payload->svcinfo, svcinfo, sizeof(payload->svcinfo));

    COPY_STRING_MEMB(svc_id, svc_id_len);

    payload_len = sizeof(*payload) + CALC_ENUM_LEN(msgtype) + CALC_PTR_LEN(svc_id_len);
    return upk_create_pkt(payload, payload_len, PKT_REPLY, PROTOCOL_VERSION);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_ack_repl(void)
{
    v0_ack_repl_t          *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REPL_ACK;

    payload_len = sizeof(*payload) + CALC_ENUM_LEN(msgtype);
    return upk_create_pkt(payload, payload_len, PKT_REPLY, PROTOCOL_VERSION);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_error_repl(char *svc_id, char *msg, upk_errlevel_t errlevel)
{
    v0_error_repl_t        *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REPL_ERROR;
    payload->errlevel = errlevel;
    COPY_STRING_MEMB(msg, msg_len);
    COPY_STRING_MEMB(svc_id, svc_id_len);

    payload_len =
        sizeof(*payload) + CALC_ENUM_LEN(msgtype) + CALC_ENUM_LEN(errlevel) + CALC_PTR_LEN(msg_len) +
        CALC_PTR_LEN(svc_id_len);
    return upk_create_pkt(payload, payload_len, PKT_REPLY, PROTOCOL_VERSION);
}


/* *******************************************************************************************************************
 * convenience functions for pubmsg's, because, why not...
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_pub_pubmsg(void)
{
    v0_publication_t       *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = PUB_PUBLICATION;

    payload_len = sizeof(*payload) + CALC_ENUM_LEN(msgtype);
    return upk_create_pkt(payload, payload_len, PKT_PUBMSG, PROTOCOL_VERSION);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_pub_cancel(void)
{
    v0_cancelation_t       *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = PUB_CANCELATION;

    payload_len = sizeof(*payload) + CALC_ENUM_LEN(msgtype);
    return upk_create_pkt(payload, payload_len, PKT_PUBMSG, PROTOCOL_VERSION);
}
