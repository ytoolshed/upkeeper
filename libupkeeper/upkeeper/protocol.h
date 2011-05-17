#ifndef _UPK_PROTOCOL_H
#define _UPK_PROTOCOL_H

/**********************************************************************************************************************
 * While in v0 this file and v0_protocol.h look nearly identical; this file serves as the public interface. 
 * and will be used exclusively elsewhere in the code. All supported versions must preserve a subset of the same args,
 * and same fields in structs as all prior supported versions. And enums must preserve order. To add support for a
 * new protocol version, create a new vN interface, add whatever additional fields and args necessary here, and add 
 * the version specific items to the switch in upk_protocol.c. Note that the underlying protocol does _not_ need
 * to maintain anything as a subset; just the wrapper function must support old and new styles.
 * this allows for controlled deprecation, while still providing protocol and API compatibility.
 **********************************************************************************************************************/
#define UPK_MAX_PACKET_SIZE 65536                          /* 64k should be enough for anyone */

#define UPK_MIN_SUPPORTED_PROTO 0
#define UPK_MAX_SUPPORTED_PROTO 0

#include "std_include.h"

/**********************************************************************************************************************
 * enums for packet type description; anything >= V0_PROTO_LIMIT is invalid * in version 0 of the protocol; 
 * future protocol extensions may be added after V0_PROTO_LIMIT in enumeration, and terminated with V1_PROTO_LIMIT,
 * etc.
 * For the sake of simplicity; these will be forced into uint32_t space; regardless of what the arch's enum size may
 * actually be
 **********************************************************************************************************************/
typedef enum {
    PKT_REQUEST = 1,
    PKT_REPLY,
    PKT_PUBMSG,
    PKT_V0_PROTO_LIMIT,
} upk_pkttype_t;

typedef enum {
    REQ_PREAMBLE = 1,
    REQ_SEQ_START,
    REQ_SEQ_END,
    REQ_ACTION,
    REQ_SIGNAL,
    REQ_LIST,
    REQ_STATUS,
    REQ_SUBSCRIBE,
    REQ_UNSUBSCRIBE,
    REQ_DISCONNECT,
    REQ_V0_PROTO_LIMIT,
} upk_req_msgtype_t;

typedef enum {
    REPL_PREAMBLE = 1,
    REPL_SEQ_START,
    REPL_SEQ_END,
    REPL_RESULT,
    REPL_LISTING,
    REPL_SVCINFO,
    REPL_ACK,
    REPL_ERROR,
    REPL_V0_PROTO_LIMIT,
} upk_reply_msgtype_t;

typedef enum {
    PUB_PUBLICATION = 1,
    PUB_CANCELATION,
    PUB_V0_PROTO_LIMIT,
} upk_pub_msgtype_t;

/**********************************************************************************************************************
 * |----|----|----|<payload ...>|----| 
 **********************************************************************************************************************/
typedef struct {
    uint32_t                version_id;
    upk_pkttype_t           pkttype;                       /* will be forced into a uint32_t; even on 64bit */
    uint32_t                payload_len;                   /* not size_t, because this would force the arch of the
                                                            * client to match the server */
    void                   *payload;
    uint32_t                crc32;
} upk_packet_t;

/**********************************************************************************************************************
 * |----|----|<client_name ...>|
 **********************************************************************************************************************/
typedef struct {
    upk_req_msgtype_t       msgtype;
    uint32_t                min_supported_ver;
    uint32_t                max_supported_ver;
    uint32_t                client_name_len;
    char                   *client_name;
} upk_req_preamble_t;

/**********************************************************************************************************************
 * |----|----|----|
 **********************************************************************************************************************/
typedef struct {
    upk_req_msgtype_t       msgtype;
    upk_req_msgtype_t       msg_seq_type;
    uint32_t                msg_seq_count;                 /* 0 if open-ended, to be terminated with a seq_end msg */
} upk_req_seq_start_t;

/**********************************************************************************************************************
 * |----|-|
 **********************************************************************************************************************/
typedef struct {
    upk_req_msgtype_t       msgtype;
    uint8_t                 commit;                        /* if false, the preceding sequence of requests since the *
                                                            * initiating seq_start will be discarded */
} upk_req_seq_end_t;

/**********************************************************************************************************************
 * |----|----|<svc_id ...>|----|<action ...>|
 **********************************************************************************************************************/
typedef struct {
    upk_req_msgtype_t       msgtype;
    uint32_t                svc_id_len;
    char                   *svc_id;
    uint32_t                action_len;
    char                   *action;
} upk_action_req_t;

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
} upk_signal_req_t;

/**********************************************************************************************************************
 * |----|
 **********************************************************************************************************************/
typedef struct {
    upk_req_msgtype_t       msgtype;
} upk_list_req_t;

/**********************************************************************************************************************
 * |----|----|----|----|<svc_id...>|
 **********************************************************************************************************************/
typedef struct {
    upk_req_msgtype_t       msgtype;
    pfieldmask_t            primary_fields;
    sfieldmask_t            secondary_fields;
    uint32_t                svc_id_len;
    char                   *svc_id;
} upk_status_req_t;

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
} upk_subscribe_req_t;

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
} upk_unsubscribe_req_t;

/**********************************************************************************************************************
 * |----|----|
 **********************************************************************************************************************/
typedef struct {
    upk_reply_msgtype_t     msgtype;
    uint32_t                best_version;
} upk_reply_preamble_t;

/**********************************************************************************************************************
 * |----|----|----|
 **********************************************************************************************************************/
typedef struct {
    upk_reply_msgtype_t     msgtype;
    upk_reply_msgtype_t     msg_seq_type;
    uint32_t                msg_seq_count;
} upk_reply_seq_start_t;

/**********************************************************************************************************************
 * |----|-|
 **********************************************************************************************************************/
typedef struct {
    upk_reply_msgtype_t     msgtype;
    uint8_t                 commit;
} upk_reply_seq_end_t;

/**********************************************************************************************************************
 * replies to signal, and action requests; and informs you if the action was successful.
 * note that if there was a protocol-type error while performing the action, that will be reported here if it was 
 * an error with the service itself; but reported as an ERROR if, for instance, the service didn't exist, or other
 * controller-level error occured.
 * |----|----|-|<msg...>|
 **********************************************************************************************************************/
typedef struct {
    upk_reply_msgtype_t     msgtype;
    uint8_t                 successful;
    uint32_t                msg_len;
    char                   *msg;
} upk_result_reply_t;

/**********************************************************************************************************************
 * These will be wrapped in PREAMBLE...COMMIT sequences
 * |----|----|<svc_id...>|
 **********************************************************************************************************************/
typedef struct {
    upk_reply_msgtype_t     msgtype;
    uint32_t                svc_id_len;
    char                   *svc_id;
} upk_listing_reply_t;

/**********************************************************************************************************************
 * |----|<svcinfo ...>|----|<svc_id...|
 **********************************************************************************************************************/
typedef struct {
    upk_reply_msgtype_t     msgtype;
    uint32_t                svcinfo_len;
    upk_svc_info_t         *svcinfo;
    uint32_t                svc_id_len;
    char                   *svc_id;
} upk_svcinfo_reply_t;

/**********************************************************************************************************************
 * for when there's really nothing else to say, but you need to assure the client you were paying attention.
 * |----|
 **********************************************************************************************************************/
typedef struct {
    upk_reply_msgtype_t     msgtype;
} upk_ack_reply_t;

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
} upk_error_reply_t;

/**********************************************************************************************************************
 * |---| This just serves as an unsolicited reply marker; which is then followed by a reply sequence wrapped
 * up in seq_start/seq_end packets.
 **********************************************************************************************************************/
typedef struct {
    upk_pub_msgtype_t       msgtype;
} upk_publication_t;

/**********************************************************************************************************************
 * |---| This just serves as an unsolicited reply marker; which is then followed by a reply sequence wrapped
 * up in seq_start/seq_end packets. the msgtype of the reply sequence can only ever be listing_reply for cancelations.
 **********************************************************************************************************************/
typedef upk_publication_t upk_cancelation_t;


/**********************************************************************************************************************
 * function prototypes for serializing, deserializing, creating, and
 * otherwise manipulating packets.
 **********************************************************************************************************************/
upk_packet_t           *upk_deserialize_packet(char *buf);
char                   *upk_serialize_packet(upk_packet_t * pkt);

upk_packet_t           *upk_create_pkt(void *payload, uint32_t payload_len, upk_pkttype_t pkttype, uint32_t proto_ver);

/**********************************************************************************************************************
 * convenience functions for requests, when its easier than building the structs yourself
 **********************************************************************************************************************/
upk_packet_t           *upk_create_req_preamble(char *client_name);

upk_packet_t           *upk_create_req_seq_start(upk_req_msgtype_t seq_type, uint32_t count);
upk_packet_t           *upk_create_req_seq_end(uint8_t commit);

upk_packet_t           *upk_create_action_req(char *svc_id, char *action);
upk_packet_t           *upk_create_signal_req(char *svc_id, uint8_t signal, uint8_t signal_sid, uint8_t signal_pgrp);
upk_packet_t           *upk_create_list_req(void);
upk_packet_t           *upk_create_status_req(char *svc_id, pfieldmask_t primary_fields, sfieldmask_t secondary_fields);
upk_packet_t           *upk_create_subscr_req(char *svc_id, pfieldmask_t primary_fields, sfieldmask_t secondary_fields,
                                              uint8_t all_svcs);
upk_packet_t           *upk_create_unsubs_req(char *svc_id, uint8_t all_svcs);


/**********************************************************************************************************************
 * convenience functions for replies, when its easier than building the structs yourself
 **********************************************************************************************************************/
upk_packet_t           *upk_create_reply_preamble(uint32_t best_version);

upk_packet_t           *upk_create_reply_seq_start(upk_reply_msgtype_t seq_type, uint32_t count);
upk_packet_t           *upk_create_reply_seq_end(uint8_t commit);

upk_packet_t           *upk_create_result_reply(char *msg, uint8_t successful);
upk_packet_t           *upk_create_listing_reply(char *svc_id);
upk_packet_t           *upk_create_svcinfo_reply(char *svc_id, upk_svc_info_t * svcinfo);
upk_packet_t           *upk_create_ack_reply(void);
upk_packet_t           *upk_create_error_reply(char *svc_id, char *errmsg, upk_errlevel_t errlvl);


/**********************************************************************************************************************
 * convenience functions for pubmsg's, because, why not...
 **********************************************************************************************************************/
upk_packet_t           *upk_create_pub_pubmsg(void);
upk_packet_t           *upk_create_pub_cancel(void);

#endif
