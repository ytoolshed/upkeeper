#include "upk_protocol.h"
#include "upk_v0_protocol.h"
#include <assert.h>

/* *******************************************************************************************************************
 * upk_create_pkt: Create a packet
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_pkt(void *payload, uint32_t payload_len, upk_pkttype_t pkttype, uint32_t proto_ver)
{
    upk_packet_t           *pkt = NULL;

    pkt = calloc(1, sizeof(*pkt));
    pkt->payload_len = payload_len;
    pkt->version_id = proto_ver;
    pkt->seq_num = 0;
    pkt->pkttype = pkttype;
    pkt->payload = payload;
    pkt->crc32 = 0;                                        /* populated/utilized by serialize/deserialize; which assert 
                                                            * validity; everything else can assume packets are valid */

    return pkt;
}


/* *******************************************************************************************************************
 * req_preamble_len: size of a preamble payload
 * ****************************************************************************************************************** */
static inline           uint32_t
req_preamble_len(upk_req_preamble_t * preamble)
{
    return (sizeof(*preamble) + preamble->client_name_len + 1);
}


/* *******************************************************************************************************************
 * upk_create_req_preamble: Create a preamble
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_req_preamble(char *client_name)
{
    upk_req_preamble_t     *preamble = NULL;

    /* FIXME: use upk_error.h */
    assert(preamble = calloc(1, sizeof(*preamble)));

    preamble->msgtype = UPK_REQ_PREAMBLE;
    preamble->min_supported_ver = UPK_MIN_SUPPORTED_PROTO;
    preamble->max_supported_ver = UPK_MAX_SUPPORTED_PROTO;
    preamble->client_name_len = strnlen(client_name, UPK_MAX_STRING_LEN);
    strncpy(preamble->client_name, client_name, preamble->client_name_len + 1);

    return upk_create_pkt(preamble, req_preamble_len(preamble), PKT_REQUEST, 0);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void
upk_free_payload(upk_packet_t * pkt)
{
    if(pkt->payload) {
        v0_free_payload(pkt);
        /* put v1_free_payload(pkt) here if/when you extend the protocol */
        free(pkt->payload);
    }
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
void
upk_pkt_free(upk_packet_t * pkt)
{
    if(pkt) {
        upk_free_payload(pkt);
        free(pkt);
    }
}


/* *******************************************************************************************************************
 * The following will make sense if/when there are multiple protocol versions being handled; right now they are just
 * wrappers
 * ****************************************************************************************************************** */

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_req_seq_start(upk_protocol_handle_t * handle, upk_msgtype_t seq_type, uint32_t count)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_req_seq_start(seq_type, count);
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_req_seq_end(upk_protocol_handle_t * handle, bool commit)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_req_seq_end(commit);
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_action_req(upk_protocol_handle_t * handle, char *svc_id, char *action)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_action_req(svc_id, action);
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_signal_req(upk_protocol_handle_t * handle, char *svc_id, upk_signal_t signal, bool signal_sid,
                      bool signal_pgrp)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_signal_req(svc_id, signal, signal_sid, signal_pgrp);
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_list_req(upk_protocol_handle_t * handle)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_list_req();
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_status_req(upk_protocol_handle_t * handle, char *svc_id)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_status_req(svc_id);
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_subscr_req(upk_protocol_handle_t * handle, char *svc_id, bool all_svcs)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_subscr_req(svc_id, all_svcs);
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_unsubs_req(upk_protocol_handle_t * handle, char *svc_id, bool all_svcs)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_unsubs_req(svc_id, all_svcs);
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_discon_req(upk_protocol_handle_t * handle)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_discon_req();
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_repl_seq_start(upk_protocol_handle_t * handle, upk_msgtype_t seq_type, uint32_t count)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_repl_seq_start(seq_type, count);
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_repl_seq_end(upk_protocol_handle_t * handle, bool commit)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_repl_seq_end(commit);
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_result_repl(upk_protocol_handle_t * handle, char *msg, bool successful)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_result_repl(msg, successful);
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_listing_repl(upk_protocol_handle_t * handle, char *svc_id)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_listing_repl(svc_id);
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_svcinfo_repl(upk_protocol_handle_t * handle, char *svc_id, upk_svcinfo_t * svcinfo)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_svcinfo_repl(svc_id, (v0_svcinfo_t *) svcinfo);
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_ack_repl(upk_protocol_handle_t * handle)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_ack_repl();
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_error_repl(upk_protocol_handle_t * handle, char *svc_id, char *errmsg, upk_errlevel_t errlvl)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_error_repl(svc_id, errmsg, errlvl);
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_pub_pubmsg(upk_protocol_handle_t * handle)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_pub_pubmsg();
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_cancel_pubmsg(upk_protocol_handle_t * handle)
{
    switch (handle->version_id) { 
        case 0:
            return v0_create_cancel_pubmsg();
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
