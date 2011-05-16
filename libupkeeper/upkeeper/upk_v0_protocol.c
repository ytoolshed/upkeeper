#include "v0_protocol.h"

#define PROTOCOL_VERSION 0
#define CALC_SIZE(M) (payload->M - sizeof(size_t))         /* the resulting payload size will have a size_t subtracted
                                                            * * from it (the pointer itself) and instead have the
                                                            * length * of data that was pointed to in its place; its
                                                            * easier to * calculate that here */

#define COPY_STRING_MEMB(MEMB, LEN) payload->LEN = strnlen(MEMB, UPK_MAX_STRING_LEN); \
                               payload->MEMB = calloc(1, payload->LEN + 1); \
                               strncpy(payload->MEMB, MEMB, payload->LEN + 1)


/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_req_seq_start(upk_req_msgtype_t msg_seq_type, uint32_t msg_seq_count)
{
    v0_req_seq_start_t     *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_SEQ_START;
    payload->msg_seq_type = msg_seq_type;
    payload->msg_seq_count = msg_seq_count;

    payload_len = sizeof(*payload);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}


/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_req_seq_end(uint8_t commit)
{
    v0_req_seq_end_t       *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_SEQ_END;
    payload->commit = commit;

    payload_len = sizeof(*payload);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}


/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_action_req(char *svc_id, char *action)
{
    v0_action_req_t        *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_ACTION;
    COPY_STRING_MEMB(svc_id, svc_id_len);
    COPY_STRING_MEMB(action, action_len);

    payload_len = sizeof(*payload) + CALC_SIZE(svc_id_len) + CALC_SIZE(action_len);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}

/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_signal_req(char *svc_id, uint8_t signal, uint8_t signal_sid, uint8_t signal_pgrp)
{
    v0_signal_req_t        *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_SIGNAL;
    payload->signal = signal;
    payload->signal_sid = signal_sid;
    payload->signal_pgrp = signal_pgrp;
    COPY_STRING_MEMB(svc_id, svc_id_len);

    payload_len = sizeof(*payload) + CALC_SIZE(svc_id_len);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}


/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_list_req(void)
{
    v0_list_req_t          *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_LIST;

    payload_len = sizeof(*payload);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}


/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_status_req(char *svc_id, pfieldmask_t primary_fields, sfieldmask_t secondary_fields)
{
    v0_status_req_t        *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_STATUS;
    payload->primary_fields = primary_fields;
    payload->secondary_fields = secondary_fields;
    COPY_STRING_MEMB(svc_id, svc_id_len);

    payload_len = sizeof(*payload) + CALC_SIZE(svc_id_len);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}


/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_subscr_req(char *svc_id, pfieldmask_t primary_fields, sfieldmask_t secondary_fields, uint8_t all_svcs)
{
    v0_subscribe_req_t     *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_SUBSCRIBE;
    payload->primary_fields = primary_fields;
    payload->secondary_fields = secondary_fields;
    COPY_STRING_MEMB(svc_id, svc_id_len);

    payload_len = sizeof(*payload) + CALC_SIZE(svc_id_len);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}



/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_unsubs_req(char *svc_id, uint8_t all_svcs)
{
    v0_unsubscribe_req_t   *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REQ_UNSUBSCRIBE;
    payload->all_svcs = all_svcs;
    COPY_STRING_MEMB(svc_id, svc_id_len);

    payload_len = sizeof(*payload) + CALC_SIZE(svc_id_len);
    return upk_create_pkt(payload, payload_len, PKT_REQUEST, PROTOCOL_VERSION);
}



/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_reply_seq_start(upk_reply_msgtype_t msg_seq_type, uint32_t msg_seq_count)
{
    v0_reply_seq_start_t   *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REPL_SEQ_START;
    payload->msg_seq_type = msg_seq_type;
    payload->msg_seq_count = msg_seq_count;

    payload_len = sizeof(*payload);
    return upk_create_pkt(payload, payload_len, PKT_REPLY, PROTOCOL_VERSION);
}

/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_reply_seq_end(uint8_t commit)
{
    v0_reply_seq_end_t     *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REPL_SEQ_END;
    payload->commit = commit;

    payload_len = sizeof(*payload);
    return upk_create_pkt(payload, payload_len, PKT_REPLY, PROTOCOL_VERSION);
}

/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_result_reply(char *msg, uint8_t successful)
{
    v0_result_reply_t      *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REPL_RESULT;
    payload->successful = successful;
    COPY_STRING_MEMB(msg, msg_len);

    payload_len = sizeof(*payload) + CALC_SIZE(msg_len);
    return upk_create_pkt(payload, payload_len, PKT_REPLY, PROTOCOL_VERSION);
}


/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_listing_reply(char *svc_id)
{
    v0_listing_reply_t     *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REPL_LISTING;
    COPY_STRING_MEMB(svc_id, svc_id_len);

    payload_len = sizeof(*payload) + CALC_SIZE(svc_id_len);
    return upk_create_pkt(payload, payload_len, PKT_REPLY, PROTOCOL_VERSION);
}

/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_svcinfo_reply(char *svc_id, upk_svc_info_t * svcinfo)
{
    v0_svcinfo_reply_t     *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REPL_SVCINFO;
    payload->svcinfo_len = get_svcinfo_len(svcinfo);
    payload->svcinfo = svcinfo;                            /* XXX Don't muck about with the data pointed at after
                                                            * creating the pkt please */
    COPY_STRING_MEMB(svc_id, svc_id_len);

    payload_len = sizeof(*payload) + CALC_SIZE(svcinfo_len) + CALC_SIZE(svc_id_len);
    return upk_create_pkt(payload, payload_len, PKT_REPLY, PROTOCOL_VERSION);
}

/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_ack_reply(void)
{
    v0_ack_reply_t         *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REPL_ACK;

    payload_len = sizeof(*payload);
    return upk_create_pkt(payload, payload_len, PKT_REPLY, PROTOCOL_VERSION);
}

/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_error_reply(char *svc_id, char *msg, upk_errlevel_t errlevel)
{
    v0_error_reply_t       *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = REPL_ERROR;
    payload->errlevel = errlevel;
    COPY_STRING_MEMB(msg, msg_len);
    COPY_STRING_MEMB(svc_id, svc_id_len);

    payload_len = sizeof(*payload) + CALC_SIZE(msg_len) + CALC_SIZE(svc_id_len);
    return upk_create_pkt(payload, payload_len, PKT_REPLY, PROTOCOL_VERSION);
}


/**********************************************************************************************************************
 * convenience functions for pubmsg's, because, why not...
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_pub_pubmsg(void)
{
    v0_publication_t       *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = PUB_PUBLICATION;

    payload_len = sizeof(*payload);
    return upk_create_pkt(payload, payload_len, PKT_PUBMSG, PROTOCOL_VERSION);
}

/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
v0_create_pub_cancel(void)
{
    v0_cancelation_t       *payload = NULL;
    uint32_t                payload_len = 0;

    payload = calloc(1, sizeof(*payload));

    payload->msgtype = PUB_CANCELATION;

    payload_len = sizeof(*payload);
    return upk_create_pkt(payload, payload_len, PKT_PUBMSG, PROTOCOL_VERSION);
}
