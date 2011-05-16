#ifndef _UPK_V0_PROTOCOL_H
#define _UPK_V0_PROTOCOL_H

#include "protocol.h"

#define V0_MAX_PACKET_SIZE 65536;                          /* 64k should be enough for anyone */

/**********************************************************************************************************************
 * |----|----|----|
 **********************************************************************************************************************/
typedef struct {
    upk_req_msgtype_t       msgtype;
    upk_req_msgtype_t       msg_seq_type;
    uint32_t                msg_seq_count;                 /* 0 if open-ended, to be terminated with a seq_end msg */
} v0_req_seq_start_t;

/**********************************************************************************************************************
 * |----|-|
 **********************************************************************************************************************/
typedef struct {
    upk_req_msgtype_t       msgtype;
    uint8_t                 commit;                        /* if false, the preceding sequence of requests since the *
                                                            * initiating seq_start will be discarded */
} v0_req_seq_end_t;

/**********************************************************************************************************************
 * |----|----|<svc_id ...>|----|<action ...>|
 **********************************************************************************************************************/
typedef struct {
    upk_req_msgtype_t       msgtype;
    uint32_t                svc_id_len;
    char                   *svc_id;
    uint32_t                action_len;
    char                   *action;
} v0_action_req_t;

/*
 * XXX: All uint8_t types will be typecast to uint8_t when serialized (they probably are already on your platform anyway)
 */

/**********************************************************************************************************************
 * |----|-|-|-|----|<svc_id ...>|
 **********************************************************************************************************************/
typedef struct {
    upk_req_msgtype_t       msgtype;
    uint8_t                 signal;
    uint8_t                 signal_sid;
    uint8_t                 signal_pgrp;
    uint32_t                svc_id_len;
    char                   *svc_id;
} v0_signal_req_t;

/**********************************************************************************************************************
 * |----|
 **********************************************************************************************************************/
typedef struct {
    upk_req_msgtype_t       msgtype;
} v0_list_req_t;

/**********************************************************************************************************************
 * |----|----|----|----|<svc_id...>|
 **********************************************************************************************************************/
typedef struct {
    upk_req_msgtype_t       msgtype;
    pfieldmask_t            primary_fields;
    sfieldmask_t            secondary_fields;
    uint32_t                svc_id_len;
    char                   *svc_id;
} v0_status_req_t;

/*
 * XXX: All uint8_t types will be typecast to uint8_t when serialized (they probably are already on your platform anyway)
 */

/**********************************************************************************************************************
 * |----|----|----|-|----|<svc_id...>|
 **********************************************************************************************************************/
typedef struct {
    upk_req_msgtype_t       msgtype;
    pfieldmask_t            primary_fields;
    sfieldmask_t            secondary_fields;
    uint8_t                 all_svcs;                      /* send a req with all=true to subscribe to everything in *
                                                            * one shot; svc_id will be ignored, and should have len 0 */
    uint32_t                svc_id_len;
    char                   *svc_id;
} v0_subscribe_req_t;

/*
 * XXX: All uint8_t types will be typecast to uint8_t when serialized (they probably are already on your platform anyway)
 */

/**********************************************************************************************************************
 * |----|-|----|<svc_id ...>|
 **********************************************************************************************************************/
typedef struct {
    upk_req_msgtype_t       msgtype;
    uint8_t                 all_svcs;
    uint32_t                svc_id_len;
    char                   *svc_id;
} v0_unsubscribe_req_t;

/**********************************************************************************************************************
 * |----|----|----|
 **********************************************************************************************************************/
typedef struct {
    upk_reply_msgtype_t     msgtype;
    upk_reply_msgtype_t     msg_seq_type;
    uint32_t                msg_seq_count;
} v0_reply_seq_start_t;

/**********************************************************************************************************************
 * |----|-|
 **********************************************************************************************************************/
typedef struct {
    upk_reply_msgtype_t     msgtype;
    uint8_t                 commit;
} v0_reply_seq_end_t;

/**********************************************************************************************************************
 * replies to signal, and action requests; and informs you if the action was successful.
 * note that if there was a protocol-type error while performing the action, that will be reported here if it was 
 * an error with the service itself; but reported as an ERROR if, for instance, the service didn't exist, or other
 * controller-level error occured.
 * |----|-|----|<msg...>|
 **********************************************************************************************************************/
typedef struct {
    upk_reply_msgtype_t     msgtype;
    uint8_t                 successful;
    uint32_t                msg_len;
    char                   *msg;
} v0_result_reply_t;

/**********************************************************************************************************************
 * These will be wrapped in PREAMBLE...COMMIT sequences
 * |----|----|<svc_id...>|
 **********************************************************************************************************************/
typedef struct {
    upk_reply_msgtype_t     msgtype;
    uint32_t                svc_id_len;
    char                   *svc_id;
} v0_listing_reply_t;

/**********************************************************************************************************************
 * |----|<svcinfo ...>|----|<svc_id...|
 **********************************************************************************************************************/
typedef struct {
    upk_reply_msgtype_t     msgtype;
    uint32_t                svcinfo_len;
    upk_svc_info_t         *svcinfo;
    uint32_t                svc_id_len;
    char                   *svc_id;
} v0_svcinfo_reply_t;

/**********************************************************************************************************************
 * for when there's really nothing else to say, but you need to assure the client you were paying attention.
 * |----|
 **********************************************************************************************************************/
typedef struct {
    upk_reply_msgtype_t     msgtype;
} v0_ack_reply_t;

/**********************************************************************************************************************
 * |----|-|----|<msg...>|
 **********************************************************************************************************************/
typedef struct {
    upk_reply_msgtype_t     msgtype;
    upk_errlevel_t          errlevel;
    uint32_t                msg_len;
    char                   *msg;
    uint32_t                svc_id_len;
    char                   *svc_id;
} v0_error_reply_t;

/**********************************************************************************************************************
 * |---| This just serves as an unsolicited reply marker; which is then followed by a reply sequence wrapped
 * up in seq_start/seq_end packets.
 **********************************************************************************************************************/
typedef struct {
    upk_pub_msgtype_t       msgtype;
} v0_publication_t;

/**********************************************************************************************************************
 * |---| This just serves as an unsolicited reply marker; which is then followed by a reply sequence wrapped
 * up in seq_start/seq_end packets. the msgtype of the reply sequence can only ever be listing_reply for cancelations.
 **********************************************************************************************************************/
typedef v0_publication_t v0_cancelation_t;


/**********************************************************************************************************************
 * function prototypes for serializing, deserializing, creating, and
 * otherwise manipulating packets.
 **********************************************************************************************************************/
upk_packet_t           *v0_deserialize_packet(char *buf);
char                   *v0_serialize_packet(upk_packet_t * pkt);

/**********************************************************************************************************************
 * convenience functions for requests, when its easier than building the structs yourself
 **********************************************************************************************************************/
upk_packet_t           *v0_create_req_seq_start(upk_req_msgtype_t msg_seq_type, uint32_t msg_seq_count);
upk_packet_t           *v0_create_req_seq_end(uint8_t commit);

upk_packet_t           *v0_create_action_req(char *svc_id, char *action);
upk_packet_t           *v0_create_signal_req(char *svc_id, uint8_t signal, uint8_t signal_sid, uint8_t signal_pgrp);
upk_packet_t           *v0_create_list_req(void);
upk_packet_t           *v0_create_status_req(char *svc_id, pfieldmask_t primary_fields, sfieldmask_t secondary_fields);
upk_packet_t           *v0_create_subscr_req(char *svc_id, pfieldmask_t primary_fields, sfieldmask_t secondary_fields,
                                             uint8_t all_svcs);
upk_packet_t           *v0_create_unsubs_req(char *svc_id, uint8_t all_svcs);


/**********************************************************************************************************************
 * convenience functions for replies, when its easier than building the structs yourself
 **********************************************************************************************************************/
upk_packet_t           *v0_create_reply_seq_start(upk_reply_msgtype_t msg_seq_type, uint32_t msg_seq_count);
upk_packet_t           *v0_create_reply_seq_end(uint8_t commit);

upk_packet_t           *v0_create_result_reply(char *msg, uint8_t successful);
upk_packet_t           *v0_create_listing_reply(char *svc_id);
upk_packet_t           *v0_create_svcinfo_reply(char *svc_id, upk_svc_info_t * svcinfo);
upk_packet_t           *v0_create_ack_reply(void);
upk_packet_t           *v0_create_error_reply(char *svc_id, char *errmsg, upk_errlevel_t errlvl);


/**********************************************************************************************************************
 * convenience functions for pubmsg's, because, why not...
 **********************************************************************************************************************/
upk_packet_t           *v0_create_pub_pubmsg(void);
upk_packet_t           *v0_create_pub_cancel(void);

#endif
