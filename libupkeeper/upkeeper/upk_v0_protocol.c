#include "v0_protocol.h"
#define PROTOCOL_VERSION 0

#define CALC_ENUM_LEN(THING) 0
#define CALC_PTR_LEN(THING) 0
#define CALC_BOOL_LEN(THING) 0

#define COPY_STRING_MEMB(MEMB, LEN) payload->LEN = strnlen(MEMB, UPK_MAX_STRING_LEN); \
                               payload->MEMB = calloc(1, payload->LEN + 1); \
                               strncpy(payload->MEMB, MEMB, payload->LEN + 1)



#define UPK_INIT_HELPER(TYPE, MSGTYPE) \
    TYPE *UPK_DATA = NULL; \
    uint32_t UPK_DATA_LEN = 0; \
    size_t UPK_STRING_LENGTH = 0; \
    UPK_DATA = calloc(1, sizeof(*UPK_DATA)); \
    UPK_DATA->msgtype = MSGTYPE

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

#define UPK_HELPER_STRING(MEMB, LEN_MEMB) \
    UPK_HELPER_ANSISTRING(UPK_DATA->MEMB, MEMB); \
    UPK_HELPER_UINT32_EXPLICIT(UPK_DATA->LEN_MEMB, UPK_STRING_LENGTH)


#define UPK_END_HELPER(PKT_TYPE) return upk_create_pkt(UPK_DATA, UPK_DATA_LEN, PKT_TYPE, PROTOCOL_VERSION)

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
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_action_req(char *svc_id, char *action)
{
    UPK_INIT_HELPER(v0_action_req_t, REQ_ACTION);

    UPK_HELPER_STRING(svc_id, svc_id_len);
    UPK_HELPER_STRING(action, action_len);

    UPK_END_HELPER(PKT_REQUEST);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_signal_req(char *svc_id, uint8_t signal, bool signal_sid, bool signal_pgrp)
{
    UPK_INIT_HELPER(v0_signal_req_t, REQ_SIGNAL);

    UPK_HELPER_ENUM(signal);
    UPK_HELPER_BOOL(signal_sid);
    UPK_HELPER_BOOL(signal_pgrp);
    UPK_HELPER_STRING(svc_id, svc_id_len);

    UPK_END_HELPER(PKT_REQUEST);
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
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_status_req(char *svc_id)
{
    UPK_INIT_HELPER(v0_status_req_t, REQ_STATUS);

    UPK_HELPER_STRING(svc_id, svc_id_len);

    UPK_END_HELPER(PKT_REQUEST);
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_subscr_req(char *svc_id, bool all_svcs)
{
    UPK_INIT_HELPER(v0_subscribe_req_t, REQ_SUBSCRIBE);

    UPK_HELPER_BOOL(all_svcs);
    UPK_HELPER_STRING(svc_id, svc_id_len);

    UPK_END_HELPER(PKT_REQUEST);
}



/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_unsubs_req(char *svc_id, bool all_svcs)
{
    UPK_INIT_HELPER(v0_unsubscribe_req_t, REQ_UNSUBSCRIBE);

    UPK_HELPER_BOOL(all_svcs);
    UPK_HELPER_STRING(svc_id, svc_id_len);

    UPK_END_HELPER(PKT_REQUEST);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_disconnect_req(void)
{
    UPK_INIT_HELPER(v0_disconnect_req_t, REQ_DISCONNECT);
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
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_result_repl(char *msg, bool successful)
{
    UPK_INIT_HELPER(v0_result_repl_t, REPL_RESULT);

    UPK_HELPER_BOOL(successful);
    UPK_HELPER_STRING(msg, msg_len);

    UPK_END_HELPER(PKT_REPLY);
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_listing_repl(char *svc_id)
{
    UPK_INIT_HELPER(v0_listing_repl_t, REPL_LISTING);

    UPK_HELPER_STRING(svc_id, svc_id_len);

    UPK_END_HELPER(PKT_REPLY);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static                  uint32_t
get_svcinfo_len(v0_svcinfo_t svcinfo)
{
    uint32_t                length = 0;

    length = sizeof(svcinfo);                              /* any non-portable types must be specially handled here */

    length += sizeof(uint32_t) - sizeof(upk_signal_name_t);
    length += sizeof(uint32_t) - sizeof(upk_state_t);
    length += sizeof(uint32_t) - sizeof(upk_state_t);
    length += svcinfo.last_action_name_len - sizeof(size_t);

    return length;
}

/* *******************************************************************************************************************
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

    UPK_HELPER_STRING(svc_id, svc_id_len);

    UPK_END_HELPER(PKT_REPLY);
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
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_error_repl(char *svc_id, char *msg, upk_errlevel_t errlevel)
{
    UPK_INIT_HELPER(v0_error_repl_t, REPL_ERROR);

    UPK_HELPER_ENUM(errlevel);
    UPK_HELPER_STRING(msg, msg_len);
    UPK_HELPER_STRING(svc_id, svc_id_len);

    UPK_END_HELPER(PKT_REPLY);
}


/* *******************************************************************************************************************
 * convenience functions for pubmsg's, because, why not...
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_pub_pubmsg(void)
{
    UPK_INIT_HELPER(v0_publication_t, PUB_PUBLICATION);
    UPK_END_HELPER(PKT_PUBMSG);
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
v0_create_pub_cancel(void)
{
    UPK_INIT_HELPER(v0_cancelation_t, PUB_CANCELATION);
    UPK_END_HELPER(PKT_PUBMSG);
}
