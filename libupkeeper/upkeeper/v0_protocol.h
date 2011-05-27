#ifndef _UPK_V0_PROTOCOL_H
#define _UPK_V0_PROTOCOL_H

#include "protocol.h"
#include "v0_protocol_structs.h"


/* *******************************************************************************************************************
 * fields defined in v0_protocol_structs.h; check there for info
 * ****************************************************************************************************************** */
typedef struct {
    UPK_V0_REQ_SEQ_START_T_FIELDS;
} v0_req_seq_start_t;

typedef struct {
    UPK_V0_REQ_SEQ_END_T_FIELDS;
} v0_req_seq_end_t;

typedef struct {
    UPK_V0_ACTION_REQ_T_FIELDS;
} v0_action_req_t;

typedef struct {
    UPK_V0_SIGNAL_REQ_T_FIELDS;
} v0_signal_req_t;

typedef struct {
    UPK_V0_LIST_REQ_T_FIELDS;
} v0_list_req_t;

typedef struct {
    UPK_V0_STATUS_REQ_T_FIELDS;
} v0_status_req_t;

typedef struct {
    UPK_V0_SUBSCR_REQ_T_FIELDS;
} v0_subscr_req_t;

typedef struct {
    UPK_V0_UNSUBS_REQ_T_FIELDS;
} v0_unsubs_req_t;

typedef struct {
    UPK_V0_DISCON_REQ_T_FIELDS;
} v0_discon_req_t;

typedef struct {
    UPK_V0_REPL_SEQ_START_T_FIELDS;
} v0_repl_seq_start_t;

typedef struct {
    UPK_V0_REPL_SEQ_END_T_FIELDS;
} v0_repl_seq_end_t;

typedef struct {
    UPK_V0_RESULT_REPL_T_FIELDS;
} v0_result_repl_t;

typedef struct {
    UPK_V0_LISTING_REPL_T_FIELDS;
} v0_listing_repl_t;

typedef struct {
    UPK_V0_SVCINFO_T_FIELDS;
} v0_svcinfo_t;

typedef struct {
    UPK_V0_SVCINFO_REPL_T_FIELDS;
} v0_svcinfo_repl_t;

typedef struct {
    UPK_V0_ACK_REPL_T_FIELDS;
} v0_ack_repl_t;

typedef struct {
    UPK_V0_ERROR_REPL_T_FIELDS;
} v0_error_repl_t;

typedef struct {
    UPK_V0_PUBLICATION_T_FIELDS;
} v0_pub_pubmsg_t;

typedef struct {
    UPK_V0_CANCELATION_T_FIELDS;
} v0_cancel_pubmsg_t;


/* *******************************************************************************************************************
 * function prototypes for serializing, deserializing, creating, and otherwise manipulating packets.
 * ****************************************************************************************************************** */
extern upk_pkt_buf_t   *v0_serialize_payload(upk_packet_t * pkt);   /* , upk_pkttype_t pkttype, size_t size); */
extern void            *v0_deserialize_payload(upk_pkt_buf_t * UPK_BUF, upk_pkttype_t pkttype);

/* *******************************************************************************************************************
 * convenience functions for requests, when its easier than building the structs yourself
 * ****************************************************************************************************************** */
extern upk_packet_t    *v0_create_req_seq_start(upk_req_msgtype_t msg_seq_type, uint32_t msg_seq_count);
extern upk_packet_t    *v0_create_req_seq_end(bool commit);

extern upk_packet_t    *v0_create_action_req(char *svc_id, char *action);
extern upk_packet_t    *v0_create_signal_req(char *svc_id, upk_signal_t signal, bool signal_sid, bool signal_pgrp);
extern upk_packet_t    *v0_create_list_req(void);
extern upk_packet_t    *v0_create_status_req(char *svc_id);
extern upk_packet_t    *v0_create_subscr_req(char *svc_id, bool all_svcs);
extern upk_packet_t    *v0_create_unsubs_req(char *svc_id, bool all_svcs);
extern upk_packet_t    *v0_create_discon_req(void);


/* *******************************************************************************************************************
 * convenience functions for replies, when its easier than building the structs yourself
 * ****************************************************************************************************************** */
extern upk_packet_t    *v0_create_repl_seq_start(upk_repl_msgtype_t msg_seq_type, uint32_t msg_seq_count);
extern upk_packet_t    *v0_create_repl_seq_end(bool commit);

extern upk_packet_t    *v0_create_result_repl(char *msg, bool successful);
extern upk_packet_t    *v0_create_listing_repl(char *svc_id);
extern upk_packet_t    *v0_create_svcinfo_repl(char *svc_id, v0_svcinfo_t * svcinfo);
extern upk_packet_t    *v0_create_ack_repl(void);
extern upk_packet_t    *v0_create_error_repl(char *svc_id, char *errmsg, upk_errlevel_t errlvl);


/* *******************************************************************************************************************
 * convenience functions for pubmsg's, because, why not...
 * ****************************************************************************************************************** */
extern upk_packet_t    *v0_create_pub_pubmsg(void);
extern upk_packet_t    *v0_create_cancel_pubmsg(void);

/* *******************************************************************************************************************
 * housekeeping
 * ****************************************************************************************************************** */
/* extern void upk_free_payload(upk_packet_t * pkt); */

#endif
