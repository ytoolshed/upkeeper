
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

/**
  @file
  @brief version 0 of the protocol between clients and controlelr
  @add_to_section protocol
  @{
  */


#include "upk_v0_protocol.h"
#define PROTOCOL_VERSION 0

#define UPK_INIT_HELPER(TYPE, MSGTYPE) \
    TYPE *UPK_DATA = NULL; \
    uint32_t UPK_DATA_LEN = 0; \
    size_t UPK_STRING_LENGTH = 0; \
    UPK_DATA = calloc(1, sizeof(*UPK_DATA)); \
    UPK_DATA->msgtype = MSGTYPE; \
    UPK_DATA_LEN += 4; \
    UPK_DATA_LEN = UPK_DATA_LEN + 0; \
    UPK_STRING_LENGTH = UPK_STRING_LENGTH + 0

#define UPK_HELPER_UINT32_EXPLICIT(DEST,SRC) \
    DEST = SRC; \
    UPK_DATA_LEN += sizeof(uint32_t)

#define UPK_HELPER_UINT32(MEMB) \
    UPK_HELPER_UINT32_EXPLICIT(UPK_DATA->MEMB, MEMB);

#define UPK_HELPER_UINT16_EXPLICIT(DEST,SRC) \
    DEST = SRC; \
    UPK_DATA_LEN += sizeof(uint16_t)

#define UPK_HELPER_UINT16(MEMB) \
    UPK_HELPER_UINT16_EXPLICIT(UPK_DATA->MEMB, MEMB);

#define UPK_HELPER_UINT8_EXPLICIT(DEST,SRC) \
    DEST = SRC; \
    UPK_DATA_LEN += sizeof(uint8_t)

#define UPK_HELPER_UINT8(MEMB) \
    UPK_HELPER_UINT8_EXPLICIT(UPK_DATA->MEMB, MEMB);

#define UPK_HELPER_ENUM(MEMB) \
    UPK_HELPER_UINT32(MEMB)

#define UPK_HELPER_BOOL(MEMB) \
    UPK_HELPER_UINT8(MEMB)

#define UPK_HELPER_BUF(DEST, SRC, LEN) \
    /* DEST = calloc(1, LEN); */ \
    memcpy(DEST,SRC,LEN); \
    UPK_DATA_LEN += LEN

#define UPK_HELPER_ANSISTRING(DEST, SRC) \
    UPK_STRING_LENGTH = strnlen(SRC, UPK_MAX_STRING_LEN); \
    UPK_HELPER_BUF(DEST, SRC, UPK_STRING_LENGTH + 1); \
    UPK_DATA_LEN--                                         /* drop the null from the byte count */

#define UPK_HELPER_STRING(MEMB) \
    UPK_HELPER_ANSISTRING(UPK_DATA->MEMB, MEMB); \
    UPK_HELPER_UINT32_EXPLICIT(UPK_DATA->UPK_MEMB_TO_LEN(MEMB), UPK_STRING_LENGTH)

#define UPK_END_HELPER(PKT_TYPE) return upk_create_pkt(UPK_DATA, UPK_DATA_LEN, PKT_TYPE, PROTOCOL_VERSION)

static void             v0_free_repl_payload(void *UPK_DATA_PTR);
static void             v0_free_req_payload(void *UPK_DATA_PTR);

void                    v0_free_payload(upk_packet_t * pkt);

typedef void            (*free_data_t) (void *);

static free_data_t      free_pkt_dispatch[] = {
    [PKT_REQUEST] = v0_free_req_payload,
    [PKT_REPLY] = v0_free_repl_payload,
    [PKT_PUBMSG] = NULL,
};

static free_data_t      free_req_dispatch[] = {
    [UPK_REQ_SEQ_START] = NULL,
    [UPK_REQ_SEQ_END] = NULL,
    [UPK_REQ_ACTION] = NULL,
    [UPK_REQ_ACTION] = NULL,
    [UPK_REQ_SIGNAL] = NULL,
    [UPK_REQ_SIGNAL] = NULL,
    [UPK_REQ_LIST] = NULL,
    [UPK_REQ_STATUS] = NULL,
    [UPK_REQ_STATUS] = NULL,
    [UPK_REQ_SUBSCRIBE] = NULL,
    [UPK_REQ_SUBSCRIBE] = NULL,
    [UPK_REQ_UNSUBSCRIBE] = NULL,
    [UPK_REQ_UNSUBSCRIBE] = NULL,
    [UPK_REQ_DISCONNECT] = NULL,
};

static free_data_t      free_repl_dispatch[] = {
    [UPK_REPL_SEQ_START] = NULL,
    [UPK_REPL_SEQ_END] = NULL,
    [UPK_REPL_RESULT] = NULL,
    [UPK_REPL_RESULT] = NULL,
    [UPK_REPL_LISTING] = NULL,
    [UPK_REPL_LISTING] = NULL,
    [UPK_REPL_SVCINFO] = NULL,
    [UPK_REPL_SVCINFO] = NULL,
    [UPK_REPL_ACK] = NULL,
    [UPK_REPL_ERROR] = NULL,
    [UPK_REPL_ERROR] = NULL,
};

static void
v0_free_req_payload(void *UPK_DATA_PTR)
{
    v0_req_seq_start_t     *UPK_DATA = (v0_req_seq_start_t *) UPK_DATA_PTR;
    free_data_t             free_func = free_req_dispatch[UPK_DATA->msgtype];

    if(free_func)
        free_func(UPK_DATA_PTR);
}

static void
v0_free_repl_payload(void *UPK_DATA_PTR)
{
    v0_repl_seq_start_t    *UPK_DATA = (v0_repl_seq_start_t *) UPK_DATA_PTR;
    free_data_t             free_func = free_repl_dispatch[UPK_DATA->msgtype];

    if(free_func)
        free_func(UPK_DATA_PTR);
}

void
v0_free_payload(upk_packet_t * pkt)
{
    free_data_t             free_func = free_pkt_dispatch[pkt->pkttype];

    if(free_func)
        free_func(pkt->payload);
}


/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_req_seq_start(upk_msgtype_t msg_seq_type, uint32_t msg_seq_count)
{
    UPK_INIT_HELPER(v0_req_seq_start_t, UPK_REQ_SEQ_START);

    UPK_HELPER_ENUM(msg_seq_type);
    UPK_HELPER_UINT32(msg_seq_count);

    UPK_END_HELPER(PKT_REQUEST);
}


/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_req_seq_end(bool commit)
{
    UPK_INIT_HELPER(v0_req_seq_end_t, UPK_REQ_SEQ_END);

    UPK_HELPER_BOOL(commit);

    UPK_END_HELPER(PKT_REQUEST);
}


/* *******************************************************************************************************************
   create
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_req_action(char *svc_id, char *action)
{
    UPK_INIT_HELPER(v0_req_action_t, UPK_REQ_ACTION);

    UPK_HELPER_STRING(svc_id);
    UPK_HELPER_STRING(action);

    UPK_END_HELPER(PKT_REQUEST);
}

/* *******************************************************************************************************************
   create
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_req_signal(char *svc_id, upk_signal_t signal, bool signal_sid, bool signal_pgrp)
{
    UPK_INIT_HELPER(v0_req_signal_t, UPK_REQ_SIGNAL);

    UPK_HELPER_ENUM(signal);
    UPK_HELPER_BOOL(signal_sid);
    UPK_HELPER_BOOL(signal_pgrp);
    UPK_HELPER_STRING(svc_id);

    UPK_END_HELPER(PKT_REQUEST);
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_req_list(void)
{
    UPK_INIT_HELPER(v0_req_list_t, UPK_REQ_LIST);
    UPK_END_HELPER(PKT_REQUEST);
}


/* *******************************************************************************************************************
   create
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_req_status(char *svc_id)
{
    UPK_INIT_HELPER(v0_req_status_t, UPK_REQ_STATUS);

    UPK_HELPER_STRING(svc_id);

    UPK_END_HELPER(PKT_REQUEST);
}

/* *******************************************************************************************************************
   create
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_req_subscribe(char *svc_id, bool all_svcs)
{
    UPK_INIT_HELPER(v0_req_subscribe_t, UPK_REQ_SUBSCRIBE);

    UPK_HELPER_BOOL(all_svcs);
    UPK_HELPER_STRING(svc_id);

    UPK_END_HELPER(PKT_REQUEST);
}

/* *******************************************************************************************************************
   create
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_req_unsubscribe(char *svc_id, bool all_svcs)
{
    UPK_INIT_HELPER(v0_req_unsubscribe_t, UPK_REQ_UNSUBSCRIBE);

    UPK_HELPER_BOOL(all_svcs);
    UPK_HELPER_STRING(svc_id);

    UPK_END_HELPER(PKT_REQUEST);
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_req_disconnect(void)
{
    UPK_INIT_HELPER(v0_req_disconnect_t, UPK_REQ_DISCONNECT);
    UPK_END_HELPER(PKT_REQUEST);
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_repl_seq_start(upk_msgtype_t msg_seq_type, uint32_t msg_seq_count)
{
    UPK_INIT_HELPER(v0_repl_seq_start_t, UPK_REPL_SEQ_START);

    UPK_HELPER_ENUM(msg_seq_type);
    UPK_HELPER_UINT32(msg_seq_count);

    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_repl_seq_end(bool commit)
{
    UPK_INIT_HELPER(v0_repl_seq_end_t, UPK_REPL_SEQ_END);

    UPK_HELPER_BOOL(commit);
    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
   create
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_repl_result(char *msg, bool successful)
{
    UPK_INIT_HELPER(v0_repl_result_t, UPK_REPL_RESULT);

    UPK_HELPER_BOOL(successful);
    UPK_HELPER_STRING(msg);

    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_repl_listing(char *svc_id)
{
    UPK_INIT_HELPER(v0_repl_listing_t, UPK_REPL_LISTING);

    UPK_HELPER_STRING(svc_id);

    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
   free
   ****************************************************************************************************************** */
/* 
   static void v0_free_listing_repl(void *UPK_DATA_PTR) { v0_repl_listing_t *UPK_DATA = (v0_repl_listing_t *)
   UPK_DATA_PTR;

   if(UPK_DATA->svc_id) free(UPK_DATA->svc_id); } */

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static                  uint32_t
get_svcinfo_len(v0_svcinfo_t s)
{
    uint32_t                l = sizeof(s);

    l += s.last_action_name_len - sizeof(s.last_action_name);
    l += sizeof(uint32_t) - sizeof(s.last_signal_name);
    l += sizeof(uint32_t) - sizeof(s.current_state);

    return l;
}

/* *******************************************************************************************************************
   create
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_repl_svcinfo(char *svc_id, v0_svcinfo_t * svcinfo)
{
    UPK_INIT_HELPER(v0_repl_svcinfo_t, UPK_REPL_SVCINFO);

    svcinfo->last_action_name_len = strnlen(svcinfo->last_action_name, UPK_MAX_STRING_LEN);

    UPK_STRING_LENGTH = get_svcinfo_len(*svcinfo);
    UPK_HELPER_UINT32_EXPLICIT(UPK_DATA->svcinfo_len, UPK_STRING_LENGTH);
    memcpy(&UPK_DATA->svcinfo, svcinfo, sizeof(*svcinfo));
    UPK_DATA_LEN += UPK_STRING_LENGTH;

    UPK_HELPER_STRING(svc_id);

    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_repl_ack(void)
{
    UPK_INIT_HELPER(v0_repl_ack_t, UPK_REPL_ACK);
    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
   create
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_repl_error(char *svc_id, upk_errno_t uerrno, char *msg, upk_errlevel_t errlevel)
{
    UPK_INIT_HELPER(v0_repl_error_t, UPK_REPL_ERROR);

    UPK_HELPER_ENUM(errlevel);
    UPK_HELPER_ENUM(uerrno);
    UPK_HELPER_STRING(msg);
    UPK_HELPER_STRING(svc_id);

    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
   convenience functions for pubmsg's, because, why not...
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_pub_publication(void)
{
    UPK_INIT_HELPER(v0_pub_publication_t, UPK_PUB_PUBLICATION);
    UPK_END_HELPER(PKT_PUBMSG);
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
v0_create_pub_cancelation(void)
{
    UPK_INIT_HELPER(v0_pub_cancelation_t, UPK_PUB_CANCELATION);
    UPK_END_HELPER(PKT_PUBMSG);
}

/**
  @}
  */
