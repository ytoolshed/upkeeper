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
    DEST = calloc(1, LEN); \
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

static void v0_free_error_repl(void *UPK_DATA_PTR);
static void v0_free_svcinfo_repl(void *UPK_DATA_PTR);
static void v0_free_listing_repl(void *UPK_DATA_PTR);
static void v0_free_result_repl(void *UPK_DATA_PTR);
static void v0_free_unsubs_req(void *UPK_DATA_PTR);
static void v0_free_subscr_req(void *UPK_DATA_PTR);
static void v0_free_status_req(void *UPK_DATA_PTR);
static void v0_free_signal_req(void *UPK_DATA_PTR);
static void v0_free_action_req(void *UPK_DATA_PTR);
static void v0_free_repl_payload(void *UPK_DATA_PTR);
static void v0_free_req_payload(void *UPK_DATA_PTR);
void v0_free_payload(upk_packet_t *pkt);

typedef void            (*free_data_t) (void *);

static free_data_t             free_pkt_dispatch[] = {
    [PKT_REQUEST] = v0_free_req_payload,
    [PKT_REPLY] = v0_free_repl_payload,
    [PKT_PUBMSG] = NULL,
};

static free_data_t             free_req_dispatch[] = {
    [REQ_SEQ_START] = NULL,
    [REQ_SEQ_END] = NULL,
    [REQ_ACTION] = v0_free_action_req,
    [REQ_SIGNAL] = v0_free_signal_req,
    [REQ_LIST] = NULL,
    [REQ_STATUS] = v0_free_status_req,
    [REQ_SUBSCRIBE] = v0_free_subscr_req,
    [REQ_UNSUBSCRIBE] = v0_free_unsubs_req,
    [REQ_DISCONNECT] = NULL,
};

static free_data_t             free_repl_dispatch[] = {
    [REPL_SEQ_START] = NULL,
    [REPL_SEQ_END] = NULL,
    [REPL_RESULT] = v0_free_result_repl,
    [REPL_LISTING] = v0_free_listing_repl,
    [REPL_SVCINFO] = v0_free_svcinfo_repl,
    [REPL_ACK] = NULL,
    [REPL_ERROR] = v0_free_error_repl,
};

static void
v0_free_req_payload(void *UPK_DATA_PTR)
{
    v0_req_seq_start_t *UPK_DATA = (v0_req_seq_start_t *) UPK_DATA_PTR;
    free_data_t free_func = free_req_dispatch[UPK_DATA->msgtype];
    if(free_func)
        free_func(UPK_DATA_PTR);
}

static void
v0_free_repl_payload(void *UPK_DATA_PTR)
{
    v0_repl_seq_start_t *UPK_DATA = (v0_repl_seq_start_t *) UPK_DATA_PTR;
    free_data_t free_func = free_repl_dispatch[UPK_DATA->msgtype];
    if(free_func)
        free_func(UPK_DATA_PTR);
}

void
v0_free_payload(upk_packet_t *pkt)
{
    free_data_t free_func = free_pkt_dispatch[pkt->pkttype];
    if(free_func)
        free_func(pkt->payload);
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_req_seq_start(upk_req_msgtype_t msg_seq_type, uint32_t msg_seq_count)
{
    UPK_INIT_HELPER(v0_req_seq_start_t, REQ_SEQ_START);

    UPK_HELPER_ENUM(msg_seq_type);
    UPK_HELPER_UINT32(msg_seq_count);

    UPK_END_HELPER(PKT_REQUEST);
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_req_seq_end(bool commit)
{
    UPK_INIT_HELPER(v0_req_seq_end_t, REQ_SEQ_END);

    UPK_HELPER_BOOL(commit);

    UPK_END_HELPER(PKT_REQUEST);
}


/* *******************************************************************************************************************
 * create
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_action_req(char *svc_id, char *action)
{
    UPK_INIT_HELPER(v0_action_req_t, REQ_ACTION);

    UPK_HELPER_STRING(svc_id);
    UPK_HELPER_STRING(action);

    UPK_END_HELPER(PKT_REQUEST);
}

/* *******************************************************************************************************************
 * free
 * ****************************************************************************************************************** */
static void
v0_free_action_req(void *UPK_DATA_PTR)
{
    v0_action_req_t        *UPK_DATA = (v0_action_req_t *) UPK_DATA_PTR;

    if(UPK_DATA->svc_id)
        free(UPK_DATA->svc_id);
    if(UPK_DATA->action)
        free(UPK_DATA->action);
}


/* *******************************************************************************************************************
 * create
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_signal_req(char *svc_id, upk_signal_t signal, bool signal_sid, bool signal_pgrp)
{
    UPK_INIT_HELPER(v0_signal_req_t, REQ_SIGNAL);

    UPK_HELPER_ENUM(signal);
    UPK_HELPER_BOOL(signal_sid);
    UPK_HELPER_BOOL(signal_pgrp);
    UPK_HELPER_STRING(svc_id);

    UPK_END_HELPER(PKT_REQUEST);
}

/* *******************************************************************************************************************
 * free
 * ****************************************************************************************************************** */
static void
v0_free_signal_req(void *UPK_DATA_PTR)
{
    v0_signal_req_t        *UPK_DATA = (v0_signal_req_t *) UPK_DATA_PTR;

    if(UPK_DATA->svc_id)
        free(UPK_DATA->svc_id);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_list_req(void)
{
    UPK_INIT_HELPER(v0_list_req_t, REQ_LIST);
    UPK_END_HELPER(PKT_REQUEST);
}


/* *******************************************************************************************************************
 * create
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_status_req(char *svc_id)
{
    UPK_INIT_HELPER(v0_status_req_t, REQ_STATUS);

    UPK_HELPER_STRING(svc_id);

    UPK_END_HELPER(PKT_REQUEST);
}

/* *******************************************************************************************************************
 * free
 * ****************************************************************************************************************** */
static void
v0_free_status_req(void *UPK_DATA_PTR)
{
    v0_status_req_t        *UPK_DATA = (v0_status_req_t *) UPK_DATA_PTR;

    if(UPK_DATA->svc_id)
        free(UPK_DATA->svc_id);
}


/* *******************************************************************************************************************
 * create
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_subscr_req(char *svc_id, bool all_svcs)
{
    UPK_INIT_HELPER(v0_subscr_req_t, REQ_SUBSCRIBE);

    UPK_HELPER_BOOL(all_svcs);
    UPK_HELPER_STRING(svc_id);

    UPK_END_HELPER(PKT_REQUEST);
}

/* *******************************************************************************************************************
 * free
 * ****************************************************************************************************************** */
static void
v0_free_subscr_req(void *UPK_DATA_PTR)
{
    v0_subscr_req_t     *UPK_DATA = (v0_subscr_req_t *) UPK_DATA_PTR;

    if(UPK_DATA->svc_id)
        free(UPK_DATA->svc_id);
}


/* *******************************************************************************************************************
 * create
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_unsubs_req(char *svc_id, bool all_svcs)
{
    UPK_INIT_HELPER(v0_unsubs_req_t, REQ_UNSUBSCRIBE);

    UPK_HELPER_BOOL(all_svcs);
    UPK_HELPER_STRING(svc_id);

    UPK_END_HELPER(PKT_REQUEST);
}

/* *******************************************************************************************************************
 * free
 * ****************************************************************************************************************** */
static void
v0_free_unsubs_req(void *UPK_DATA_PTR)
{
    v0_unsubs_req_t   *UPK_DATA = (v0_unsubs_req_t *) UPK_DATA_PTR;

    if(UPK_DATA->svc_id)
        free(UPK_DATA->svc_id);
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_discon_req(void)
{
    UPK_INIT_HELPER(v0_discon_req_t, REQ_DISCONNECT);
    UPK_END_HELPER(PKT_REQUEST);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_repl_seq_start(upk_repl_msgtype_t msg_seq_type, uint32_t msg_seq_count)
{
    UPK_INIT_HELPER(v0_repl_seq_start_t, REPL_SEQ_START);

    UPK_HELPER_ENUM(msg_seq_type);
    UPK_HELPER_UINT32(msg_seq_count);

    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_repl_seq_end(bool commit)
{
    UPK_INIT_HELPER(v0_repl_seq_end_t, REPL_SEQ_END);

    UPK_HELPER_BOOL(commit);
    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
 * create
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_result_repl(char *msg, bool successful)
{
    UPK_INIT_HELPER(v0_result_repl_t, REPL_RESULT);

    UPK_HELPER_BOOL(successful);
    UPK_HELPER_STRING(msg);

    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
 * free
 * ****************************************************************************************************************** */
static void
v0_free_result_repl(void *UPK_DATA_PTR)
{
    v0_result_repl_t       *UPK_DATA = (v0_result_repl_t *) UPK_DATA_PTR;

    if(UPK_DATA->msg)
        free(UPK_DATA->msg);
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_listing_repl(char *svc_id)
{
    UPK_INIT_HELPER(v0_listing_repl_t, REPL_LISTING);

    UPK_HELPER_STRING(svc_id);

    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
 * free
 * ****************************************************************************************************************** */
static void
v0_free_listing_repl(void *UPK_DATA_PTR)
{
    v0_listing_repl_t      *UPK_DATA = (v0_listing_repl_t *) UPK_DATA_PTR;

    if(UPK_DATA->svc_id)
        free(UPK_DATA->svc_id);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static uint32_t
get_svcinfo_len(v0_svcinfo_t s)
{
    uint32_t                l = sizeof(s);

    l += s.last_action_name_len - sizeof(s.last_action_name);
    l += sizeof(uint32_t) - sizeof(s.last_signal_name);
    l += sizeof(uint32_t) - sizeof(s.current_state);

    return l;
}

/* *******************************************************************************************************************
 * create
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_svcinfo_repl(char *svc_id, v0_svcinfo_t * svcinfo)
{
    UPK_INIT_HELPER(v0_svcinfo_repl_t, REPL_SVCINFO);

    svcinfo->last_action_name_len = strnlen(svcinfo->last_action_name, UPK_MAX_STRING_LEN);

    UPK_STRING_LENGTH = get_svcinfo_len(*svcinfo);
    UPK_HELPER_UINT32_EXPLICIT(UPK_DATA->svcinfo_len, UPK_STRING_LENGTH);
    memcpy(&UPK_DATA->svcinfo, svcinfo, sizeof(*svcinfo));
    UPK_DATA_LEN += UPK_STRING_LENGTH;

    UPK_HELPER_STRING(svc_id);

    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
 * free
 * ****************************************************************************************************************** */
static void
v0_free_svcinfo_repl(void *UPK_DATA_PTR)
{
    v0_svcinfo_repl_t      *UPK_DATA = (v0_svcinfo_repl_t *) UPK_DATA_PTR;

 /*   if(UPK_DATA->svcinfo.last_action_name)
        free(UPK_DATA->svcinfo.last_action_name); */
    if(UPK_DATA->svc_id)
        free(UPK_DATA->svc_id);
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_ack_repl(void)
{
    UPK_INIT_HELPER(v0_ack_repl_t, REPL_ACK);
    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
 * create
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_error_repl(char *svc_id, char *msg, upk_errlevel_t errlevel)
{
    UPK_INIT_HELPER(v0_error_repl_t, REPL_ERROR);

    UPK_HELPER_ENUM(errlevel);
    UPK_HELPER_STRING(msg);
    UPK_HELPER_STRING(svc_id);

    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
 * free
 * ****************************************************************************************************************** */
static void
v0_free_error_repl(void *UPK_DATA_PTR)
{
    v0_error_repl_t        *UPK_DATA = (v0_error_repl_t *) UPK_DATA_PTR;

    if(UPK_DATA->msg)
        free(UPK_DATA->msg);
    if(UPK_DATA->svc_id)
        free(UPK_DATA->svc_id);
}


/* *******************************************************************************************************************
 * convenience functions for pubmsg's, because, why not...
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_pub_pubmsg(void)
{
    UPK_INIT_HELPER(v0_pub_pubmsg_t, PUB_PUBLICATION);
    UPK_END_HELPER(PKT_PUBMSG);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_cancel_pubmsg(void)
{
    UPK_INIT_HELPER(v0_cancel_pubmsg_t, PUB_CANCELATION);
    UPK_END_HELPER(PKT_PUBMSG);
}
