
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


#include "upk_protocol.h"
#include "upk_v0_protocol.h"

/* *******************************************************************************************************************
   upk_create_pkt: Create a packet
   ****************************************************************************************************************** */
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
    pkt->crc32 = 0;                                        /* populated/utilized by serialize/deserialize;
                                                              which assert validity; everything else can
                                                              assume packets are valid */

    return pkt;
}


/* *******************************************************************************************************************
   req_preamble_len: size of a preamble payload
   ****************************************************************************************************************** */
static inline           uint32_t
req_preamble_len(upk_req_preamble_t * preamble)
{
    return ((sizeof(*preamble) - sizeof(preamble->client_name)) + preamble->client_name_len);
}


/* *******************************************************************************************************************
   upk_create_req_preamble: Create a preamble
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_req_preamble(upk_protocol_handle_t * handle, char *client_name)
{
    upk_req_preamble_t     *preamble = NULL;

    preamble = calloc(1, sizeof(*preamble));

    preamble->msgtype = UPK_REQ_PREAMBLE;
    preamble->min_supported_ver = UPK_MIN_SUPPORTED_PROTO;
    preamble->max_supported_ver = UPK_MAX_SUPPORTED_PROTO;
    preamble->client_name_len = strnlen(client_name, UPK_MAX_STRING_LEN);
    strncpy(preamble->client_name, client_name, preamble->client_name_len + 1);

    return upk_create_pkt(preamble, req_preamble_len(preamble), PKT_REQUEST, 0);
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_repl_preamble(upk_protocol_handle_t * handle, uint32_t best_version)
{
    upk_repl_preamble_t    *preamble = NULL;

    preamble = calloc(1, sizeof(*preamble));

    preamble->msgtype = UPK_REPL_PREAMBLE;
    preamble->best_version = best_version;

    return upk_create_pkt(preamble, 4 + 4, PKT_REPLY, 0);
}



/* *******************************************************************************************************************
   ****************************************************************************************************************** */
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
   ****************************************************************************************************************** */
void
upk_pkt_free(upk_packet_t * pkt)
{
    if(pkt) {
        upk_free_payload(pkt);
        free(pkt);
    }
}


/* *******************************************************************************************************************
   The following will make sense if/when there are multiple protocol versions being handled; right now they are just
   wrappers
   ****************************************************************************************************************** */

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
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
   ****************************************************************************************************************** */
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
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_req_action(upk_protocol_handle_t * handle, char *svc_id, char *action)
{
    switch (handle->version_id) {
    case 0:
        return v0_create_req_action(svc_id, action);
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_req_signal(upk_protocol_handle_t * handle, char *svc_id, upk_signal_t signal, bool signal_sid,
                      bool signal_pgrp)
{
    switch (handle->version_id) {
    case 0:
        return v0_create_req_signal(svc_id, signal, signal_sid, signal_pgrp);
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_req_list(upk_protocol_handle_t * handle)
{
    switch (handle->version_id) {
    case 0:
        return v0_create_req_list();
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_req_status(upk_protocol_handle_t * handle, char *svc_id)
{
    switch (handle->version_id) {
    case 0:
        return v0_create_req_status(svc_id);
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_req_subscribe(upk_protocol_handle_t * handle, char *svc_id, bool all_svcs)
{
    switch (handle->version_id) {
    case 0:
        return v0_create_req_subscribe(svc_id, all_svcs);
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_req_unsubscribe(upk_protocol_handle_t * handle, char *svc_id, bool all_svcs)
{
    switch (handle->version_id) {
    case 0:
        return v0_create_req_unsubscribe(svc_id, all_svcs);
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_req_disconnect(upk_protocol_handle_t * handle)
{
    switch (handle->version_id) {
    case 0:
        return v0_create_req_disconnect();
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
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
   ****************************************************************************************************************** */
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
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_repl_result(upk_protocol_handle_t * handle, char *msg, bool successful)
{
    switch (handle->version_id) {
    case 0:
        return v0_create_repl_result(msg, successful);
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_repl_listing(upk_protocol_handle_t * handle, char *svc_id)
{
    switch (handle->version_id) {
    case 0:
        return v0_create_repl_listing(svc_id);
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_repl_svcinfo(upk_protocol_handle_t * handle, char *svc_id, upk_svcinfo_t * svcinfo)
{
    switch (handle->version_id) {
    case 0:
        return v0_create_repl_svcinfo(svc_id, (v0_svcinfo_t *) svcinfo);
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_repl_ack(upk_protocol_handle_t * handle)
{
    switch (handle->version_id) {
    case 0:
        return v0_create_repl_ack();
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_repl_error(upk_protocol_handle_t * handle, char *svc_id, upk_errno_t uerrno, char *errmsg,
                      upk_errlevel_t errlvl)
{
    switch (handle->version_id) {
    case 0:
        return v0_create_repl_error(svc_id, uerrno, errmsg, errlvl);
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_pub_publication(upk_protocol_handle_t * handle)
{
    switch (handle->version_id) {
    case 0:
        return v0_create_pub_publication();
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
upk_create_pub_cancelation(upk_protocol_handle_t * handle)
{
    switch (handle->version_id) {
    case 0:
        return v0_create_pub_cancelation();
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_msgtype_t
upk_get_msgtype(upk_packet_t * pkt)
{
    upk_generic_msg_t      *dummy = pkt->payload;

    return dummy->msgtype;
}


/* *******************************************************************************************************************
   ****************************************************************************************************************** */
size_t
upk_get_msgsize(upk_msgtype_t type)
{
    static size_t           msgsizes[] = {
        sizeof(upk_req_preamble_t),
        sizeof(upk_req_seq_start_t),
        sizeof(upk_req_seq_end_t),
        sizeof(upk_req_action_t),
        sizeof(upk_req_signal_t),
        sizeof(upk_req_list_t),
        sizeof(upk_req_status_t),
        sizeof(upk_req_subscribe_t),
        sizeof(upk_req_unsubscribe_t),
        sizeof(upk_req_disconnect_t),
        sizeof(upk_repl_preamble_t),
        sizeof(upk_repl_seq_start_t),
        sizeof(upk_repl_seq_end_t),
        sizeof(upk_repl_result_t),
        sizeof(upk_repl_listing_t),
        sizeof(upk_repl_svcinfo_t),
        sizeof(upk_repl_ack_t),
        sizeof(upk_repl_error_t),
        sizeof(upk_pub_publication_t),
        sizeof(upk_pub_cancelation_t),
    };
    if((UPK_MSGTYPE_IDX(type) >= 0) && (UPK_MSGTYPE_IDX(type) < (sizeof(msgsizes) / sizeof(*msgsizes))))
        return msgsizes[UPK_MSGTYPE_IDX(type)];
    return 0;
}
