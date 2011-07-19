#ifndef _UPK_PROTOCOL_H
#define _UPK_PROTOCOL_H

/**
  @file
  @brief UPK networking protocol.

  The network protocol defined herein is intended at this time for localhost communication via domain socket.
  If for any reason it is ever desirable to use this over the wire, care has been taken to ensure endian and type
  safety for transport; however, my current opinion is anything for network transport should require authentication
  and presumably service-to-service authentication, proxy authentication, etc, i.e. x509 or preferably krb5 or similar
  mechanisms, along with non-struct-buffer based communication. And so it is my contention that it would
  probably be far saner to build a network service on top of this library, implementing its own authentication 
  and communication protocol (probably utilizing json/yaml/xml/protobufs/whatever)
  */

/**
  @addtogroup client_protocol
  @{
  */


#include "upk_error.h"
#include "upk_types.h"

/* ********************************************************************************************************************
   enums must preserve order. To add support for a new protocol version, create a new vN interface, add whatever
   additional fields necessary to a vN_protocol_structs.h and append the macro in the appropriate location here.  and
   add the version specific items to the switch in upk_protocol.c. Note that the underlying protocol does _not_ need to
   maintain anything as a subset; just the wrapper function must support old and new styles.  this allows for controlled 
   deprecation, while still providing protocol and API compatibility.
   ******************************************************************************************************************* */

#define UPK_MAX_PACKET_SIZE 65536                          /*!< 64k should be enough for anyone */
#define UPK_PACKET_HEADER_LEN 16

#define UPK_MIN_SUPPORTED_PROTO 0
#define UPK_MAX_SUPPORTED_PROTO 0

#define UPK_DATA __upk_data
#define UPK_DATA_PTR __upk_data_ptr
#define UPK_DATA_LEN __upk_data_len
#define UPK_BUF __upk_buf
#define UPK_BUF_PTR __upk_buf_ptr
#define UPK_UINT8_BUFFER __upk_uint8_buf
#define UPK_UINT16_BUFFER __upk_uint16_buf
#define UPK_UINT32_BUFFER __upk_uint32_buf
#define UPK_STRING_LENGTH __upk_string_len

/* XXX you will want to redefine these depending on the type of data you're dealing with */
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_pkttype_t
#define UPK_MSG_IDENTIFIER pkttype

#define UPK_INIT_DESERIALIZE(TYPE) \
    TYPE * UPK_DATA = NULL; \
    unsigned char * UPK_BUF_PTR = UPK_BUF; \
    uint32_t UPK_UINT32_BUFFER = 0; \
    uint16_t UPK_UINT16_BUFFER = 0; \
    uint8_t UPK_UINT8_BUFFER = 0; \
    UPK_UINT32_BUFFER = UPK_UINT32_BUFFER + 0; \
    UPK_UINT16_BUFFER = UPK_UINT16_BUFFER + 0; \
    UPK_UINT8_BUFFER = UPK_UINT8_BUFFER + 0


#define UPK_MEMB_TO_LEN(MEMB) MEMB ## _len

#define UPK_INIT_DESERIALIZE_MSG(TYPE) \
    UPK_INIT_DESERIALIZE(TYPE); \
    UPK_DATA = calloc(1,sizeof(*UPK_DATA)); \
    UPK_FETCH_ENUM(UPK_MSG_IDENTIFIER_TYPEDEF, UPK_MSG_IDENTIFIER)

#define UPK_FETCH_UINT32_T(TYPE,MEMB) \
    memcpy(&UPK_UINT32_BUFFER, UPK_BUF_PTR, sizeof(UPK_UINT32_BUFFER)); \
    UPK_DATA->MEMB = (TYPE) ntohl( UPK_UINT32_BUFFER ); \
    UPK_BUF_PTR += sizeof(UPK_UINT32_BUFFER)

#define UPK_FETCH_UINT32(MEMB) \
    UPK_FETCH_UINT32_T(uint32_t, MEMB)

#define UPK_FETCH_UINT16_T(TYPE,MEMB) \
    memcpy(&UPK_UINT16_BUFFER, UPK_BUF_PTR, sizeof(UPK_UINT16_BUFFER)); \
    UPK_DATA->MEMB = (TYPE) ntohs( UPK_UINT16_BUFFER ); \
    UPK_BUF_PTR += sizeof(UPK_UINT16_BUFFER)

#define UPK_FETCH_UINT16(MEMB) \
    UPK_FETCH_UINT16_T(uint16_t, MEMB)

#define UPK_FETCH_UINT8_T(TYPE,MEMB) \
    memcpy(&UPK_UINT8_BUFFER, UPK_BUF_PTR, sizeof(UPK_UINT8_BUFFER)); \
    UPK_DATA->MEMB = (TYPE) UPK_UINT8_BUFFER; \
    UPK_BUF_PTR += sizeof(UPK_UINT8_BUFFER)

#define UPK_FETCH_UINT8(MEMB) \
    UPK_FETCH_UINT8_T(uint8_t, MEMB)

#define UPK_FETCH_BOOL(MEMB) \
    UPK_FETCH_UINT8_T(bool, MEMB)

#define UPK_FETCH_ENUM(TYPE,MEMB) \
    UPK_FETCH_UINT32_T(TYPE, MEMB)

#define UPK_FETCH_STRING(MEMB) \
    UPK_DATA->MEMB = calloc(1, UPK_DATA->UPK_MEMB_TO_LEN(MEMB) + 1);  /* null terminate */ \
    memcpy(UPK_DATA->MEMB, UPK_BUF_PTR, UPK_DATA->UPK_MEMB_TO_LEN(MEMB)); \
    UPK_BUF_PTR += UPK_DATA->UPK_MEMB_TO_LEN(MEMB)

#define UPK_FETCH_DATA_TO_BUF(BUF, LEN) \
    BUF = calloc(1, UPK_DATA->LEN); \
    memcpy(BUF, UPK_BUF_PTR, UPK_DATA->LEN); \
    UPK_BUF_PTR += UPK_DATA->LEN

#define UPK_FETCH_DATA(MEMB) \
    UPK_FETCH_DATA_TO_BUF(UPK_DATA->MEMB, UPK_MEMB_TO_LEN(MEMB))

#define UPK_FETCH_ARRAY(MEMB, LEN) \
    memcpy(UPK_DATA->MEMB, UPK_BUF_PTR, LEN); \
    UPK_BUF_PTR += LEN

#define UPK_INIT_SERIALIZE(TYPE) \
    TYPE * UPK_DATA = (TYPE *) UPK_DATA_PTR; \
    upk_pkt_buf_t * UPK_BUF = NULL; \
    upk_pkt_buf_t * UPK_BUF_PTR = NULL; \
    uint32_t UPK_UINT32_BUFFER = 0; \
    uint16_t UPK_UINT16_BUFFER = 0; \
    uint8_t UPK_UINT8_BUFFER = 0; \
    size_t UPK_STRING_LENGTH = 0; \
    UPK_UINT32_BUFFER = UPK_UINT32_BUFFER + 0; \
    UPK_UINT16_BUFFER = UPK_UINT16_BUFFER + 0; \
    UPK_UINT8_BUFFER = UPK_UINT8_BUFFER + 0; \
    UPK_STRING_LENGTH = UPK_STRING_LENGTH + 0

#define UPK_INIT_SERIALIZE_BUF(TYPE, LEN) \
    UPK_INIT_SERIALIZE(TYPE); \
    UPK_BUF = calloc(1, LEN); \
    UPK_BUF_PTR = UPK_BUF

#define UPK_INIT_SERIALIZE_MSG(TYPE) \
    UPK_INIT_SERIALIZE_BUF(TYPE, UPK_DATA_LEN); \
    UPK_PUT_ENUM(UPK_MSG_IDENTIFIER)

#define UPK_PUT_UINT32(MEMB) \
    UPK_UINT32_BUFFER = htonl( (uint32_t) UPK_DATA->MEMB ); \
    memcpy(UPK_BUF_PTR, &UPK_UINT32_BUFFER, sizeof(UPK_UINT32_BUFFER)); \
    UPK_BUF_PTR += sizeof(UPK_UINT32_BUFFER)

#define UPK_PUT_UINT16(MEMB) \
    UPK_UINT16_BUFFER = htons( (uint16_t) UPK_DATA->MEMB ); \
    memcpy(UPK_BUF_PTR, &UPK_UINT16_BUFFER, sizeof(UPK_UINT16_BUFFER)); \
    UPK_BUF_PTR += sizeof(UPK_UINT16_BUFFER)

#define UPK_PUT_UINT8(MEMB) \
    UPK_UINT8_BUFFER = (uint8_t) UPK_DATA->MEMB; \
    memcpy(UPK_BUF_PTR, &UPK_UINT8_BUFFER, sizeof(UPK_UINT8_BUFFER)); \
    UPK_BUF_PTR += sizeof(UPK_UINT8_BUFFER)

#define UPK_PUT_BOOL(MEMB) \
    UPK_PUT_UINT8(MEMB)

#define UPK_PUT_ENUM(MEMB) \
    UPK_PUT_UINT32(MEMB)

#define UPK_PUT_STRING(MEMB) \
    UPK_STRING_LENGTH = strnlen(UPK_DATA->MEMB, UPK_MAX_STRING_LEN); \
    memcpy(UPK_BUF_PTR, UPK_DATA->MEMB, strnlen(UPK_DATA->MEMB, UPK_STRING_LENGTH)); \
    UPK_BUF_PTR += UPK_STRING_LENGTH

#define UPK_PUT_DATA_FROM_BUF(BUF, LEN) \
    memcpy(UPK_BUF_PTR, BUF, UPK_DATA->LEN); \
    UPK_BUF_PTR += UPK_DATA->LEN

#define UPK_PUT_DATA(MEMB) \
    UPK_PUT_DATA_FROM_BUF(UPK_DATA->MEMB, UPK_MEMB_TO_LEN(MEMB))

#define UPK_PUT_ARRAY(MEMB, LEN) \
    memcpy(UPK_BUF_PTR, UPK_DATA->MEMB, LEN); \
    UPK_BUF_PTR += LEN

#ifdef __NEVERDEFINED_NEED_FOR_AUTOSCAN_TO_SEE_FUNCS_USED_IN_ABOVE_MACROS
static inline void
__never_compiled(void)
{
    void                   *foo, *bar;
    int32_t                 baz = 1;

    foo = calloc(1, sizeof(int32_t));
    bar = calloc(1, sizeof(int32_t));
    memcpy(foo, bar, sizeof(int32_t));
    baz = strnlen("foobar", 64);
    baz = htonl(baz);
    baz = ntohl(baz);
    baz = (int32_t) htons((int16_t) baz);
    baz = (int32_t) ntohs((int16_t) baz);
}
#endif

#include "upk_std_include.h"
#include "upk_v0_protocol_structs.h"

typedef unsigned char   upk_pkt_buf_t;

/* *******************************************************************************************************************
   enums for packet type description; anything >= V0_PROTO_LIMIT is invalid * in version 0 of the protocol; future
   protocol extensions may be added after V0_PROTO_LIMIT in enumeration, and terminated with V1_PROTO_LIMIT, etc. For
   the sake of simplicity; these will be forced into uint32_t space; regardless of what the arch's enum size may
   actually be
   ****************************************************************************************************************** */
typedef enum {
    PKT_REQUEST = 1,
    PKT_REPLY,
    PKT_PUBMSG,
    PKT_V0_PROTO_LIMIT,
} upk_pkttype_t;

typedef enum _upk_msgtype {
    UPK_REQ_PREAMBLE = 1,                                  /*!< A preamble message, used to negotiate version, and
                                                              handshake */
    UPK_REQ_SEQ_START,                                     /*!< The start of a request sequence */
    UPK_REQ_SEQ_END,                                       /*!< the end of a request sequence */
    UPK_REQ_ACTION,                                        /*!< An action request (e.g. start, stop, reload, or custom 
                                                              actions) */
    UPK_REQ_SIGNAL,                                        /*!< a signal request (e.g. send signal N) */
    UPK_REQ_LIST,                                          /*!< A listing request. for a listing of all service-ids
                                                              known to the controller */
    UPK_REQ_STATUS,                                        /*!< A status request, to get information on a particular
                                                              service */
    UPK_REQ_SUBSCRIBE,                                     /*!< subscribe to a feed of status updates */
    UPK_REQ_UNSUBSCRIBE,                                   /*!< unsubscribe from a feed of status updates */
    UPK_REQ_DISCONNECT,                                    /*!< notify controller of your intent to disconnect */
    UPK_REQ_UNCONFIGURE,                                   /*!< notify controller to unconfigure an existing service */
    UPK_REQ_V0_PROTO_LIMIT,                                /*!< all valid v0 requests are < this */
    UPK_REPL_PREAMBLE = 4096,                              /*!< the reply to a preamble request, used to send back
                                                              what the server decided was the best protocol, and
                                                              complete the handshake */
    UPK_REPL_SEQ_START,                                    /*!< the start of a sequence of replies */
    UPK_REPL_SEQ_END,                                      /*!< the end of a sequence of replies */
    UPK_REPL_RESULT,                                       /*!< the result of an action or signal request (or anything 
                                                              else that might benefit from a result msg */
    UPK_REPL_LISTING,                                      /*!< the name of a service in a sequence listing all
                                                              services */
    UPK_REPL_SVCINFO,                                      /*!< all the information known about a service */
    UPK_REPL_ACK,                                          /*!< an ack, when nothing else really fits, but a reply is
                                                              still mandated */
    UPK_REPL_ERROR,                                        /*!< report an error to the client, for instance, if the
                                                              named service doesn't exist, or an action doesn't exist,
                                                              etc */
    UPK_REPL_V0_PROTO_LIMIT,                               /*!< All valid v0 replies are < this */
    UPK_PUB_PUBLICATION = 8192,                            /*!< a message sent from the controller to a subscriber,
                                                              followed by svcinfo packets for all subscribed services */
    UPK_PUB_CANCELATION,                                   /*!< notification that a particular service is no longer
                                                              available to subscribe to, for instance if its been
                                                              removed */
    UPK_PUB_V0_PROTO_LIMIT,                                /*!< All valid pub v0 publication messages are < this */
} upk_msgtype_t;

/* *******************************************************************************************************************
   |----|----|----|<payload ...>|----|
   ****************************************************************************************************************** */
typedef struct {
    uint32_t                payload_len;                   /* not size_t, because this would force the arch of the
                                                              client to match the server; and even on localhost, that
                                                              cannot be guaranteed */
    uint32_t                version_id;
    uint32_t                seq_num;
    upk_pkttype_t           pkttype;                       /* will be forced into a uint32_t; even on 64bit */
    void                   *payload;
    uint32_t                crc32;
} upk_packet_t;

/* *******************************************************************************************************************
   |----|----|<client_name ...>|
   ****************************************************************************************************************** */
typedef struct {
    upk_msgtype_t           msgtype;
    uint32_t                min_supported_ver;
    uint32_t                max_supported_ver;
    uint32_t                client_name_len;
    char                   *client_name;
} upk_req_preamble_t;

/* *******************************************************************************************************************
   |----|----|
   ****************************************************************************************************************** */
typedef struct {
    upk_msgtype_t           msgtype;
    uint32_t                best_version;
} upk_repl_preamble_t;

/* *******************************************************************************************************************
   fields defined in vN_protocol_structs.h; check them for info
   ****************************************************************************************************************** */
typedef struct {
    UPK_V0_UPK_REQ_SEQ_START_T_FIELDS;
} upk_req_seq_start_t;

typedef struct {
    UPK_V0_UPK_REQ_SEQ_END_T_FIELDS;
} upk_req_seq_end_t;

typedef struct {
    UPK_V0_ACTION_REQ_T_FIELDS;
} upk_action_req_t;

typedef struct {
    UPK_V0_SIGNAL_REQ_T_FIELDS;
} upk_signal_req_t;

typedef struct {
    UPK_V0_LIST_REQ_T_FIELDS;
} upk_list_req_t;

typedef struct {
    UPK_V0_STATUS_REQ_T_FIELDS;
} upk_status_req_t;

typedef struct {
    UPK_V0_SUBSCR_REQ_T_FIELDS;
} upk_subscr_req_t;

typedef struct {
    UPK_V0_UNSUBS_REQ_T_FIELDS;
} upk_unsubs_req_t;

typedef struct {
    UPK_V0_DISCON_REQ_T_FIELDS;
} upk_discon_req_t;

typedef struct {
    UPK_V0_UPK_REPL_SEQ_START_T_FIELDS;
} upk_repl_seq_start_t;

typedef struct {
    UPK_V0_UPK_REPL_SEQ_END_T_FIELDS;
} upk_repl_seq_end_t;

typedef struct {
    UPK_V0_RESULT_REPL_T_FIELDS;
} upk_result_repl_t;

typedef struct {
    UPK_V0_LISTING_REPL_T_FIELDS;
} upk_listing_repl_t;

/* Moved definition to types.h */
/* typedef struct { UPK_V0_SVCINFO_T_FIELDS; } upk_svcinfo_t; */

/* redefine v0_svcinfo_t with upk_svcinfo_t so we can borrow the struct definition while still referencing the correct, 
   version-agnostic, structure therein */
#define v0_svcinfo_t upk_svcinfo_t
typedef struct {
    UPK_V0_SVCINFO_REPL_T_FIELDS;
} upk_svcinfo_repl_t;

#undef v0_svcinfo_t

typedef struct {
    UPK_V0_ACK_REPL_T_FIELDS;
} upk_ack_repl_t;

typedef struct {
    UPK_V0_ERROR_REPL_T_FIELDS;
} upk_error_repl_t;

typedef struct {
    UPK_V0_PUBLICATION_T_FIELDS;
} upk_pub_pubmsg_t;

typedef struct {
    UPK_V0_CANCELATION_T_FIELDS;
} upk_cancel_pubmsg_t;

typedef struct {
    uint32_t                version_id;
    uint32_t                seq_num;
    int                     sockfd;
} upk_protocol_handle_t;

typedef union _upk_payload_types {
    upk_req_preamble_t      req_preamble;
    upk_repl_preamble_t     repl_preamble;
    upk_req_seq_start_t     req_seq_start;
    upk_req_seq_end_t       req_seq_end;
    upk_action_req_t        action_req;
    upk_signal_req_t        signal_req;
    upk_list_req_t          list_req;
    upk_status_req_t        status_req;
    upk_subscr_req_t        subscr_req;
    upk_unsubs_req_t        unsubs_req;
    upk_discon_req_t        discon_req;
    upk_repl_seq_start_t    repl_seq_start;
    upk_repl_seq_end_t      repl_seq_end;
    upk_result_repl_t       result_repl;
    upk_listing_repl_t      listing_repl;
    upk_svcinfo_t           svcinfo;
    upk_svcinfo_repl_t      svcinfo_repl;
    upk_ack_repl_t          ack_repl;
    upk_error_repl_t        error_repl;
    upk_pub_pubmsg_t        pub_pubmsg;
    upk_cancel_pubmsg_t     cancel_pubmsg;
} upk_payload_types_u;

typedef struct _upk_payload {
    upk_msgtype_t           type;
    upk_payload_types_u     payload;
} upk_paylaod_t;


/* *******************************************************************************************************************
   function prototypes for serializing, deserializing, creating, and otherwise manipulating packets.
   ****************************************************************************************************************** */
extern upk_pkt_buf_t   *upk_serialize_packet(upk_packet_t * UPK_DATA_PTR);
extern upk_packet_t    *upk_deserialize_packet(upk_pkt_buf_t * UPK_BUF);

extern upk_packet_t    *upk_create_pkt(void *payload, uint32_t payload_len, upk_pkttype_t pkttype, uint32_t proto_ver);

/* *******************************************************************************************************************
   convenience functions for requests, when its easier than building the structs yourself
   ****************************************************************************************************************** */
extern upk_packet_t    *upk_create_req_preamble(char *client_name);

extern upk_packet_t    *upk_create_req_seq_start(upk_protocol_handle_t * handle, upk_msgtype_t seq_type,
                                                 uint32_t count);
extern upk_packet_t    *upk_create_req_seq_end(upk_protocol_handle_t * handle, bool commit);

extern upk_packet_t    *upk_create_action_req(upk_protocol_handle_t * handle, char *svc_id, char *action);
extern upk_packet_t    *upk_create_signal_req(upk_protocol_handle_t * handle, char *svc_id, upk_signal_t signal,
                                              bool signal_sid, bool signal_pgrp);
extern upk_packet_t    *upk_create_list_req(upk_protocol_handle_t * handle);
extern upk_packet_t    *upk_create_status_req(upk_protocol_handle_t * handle, char *svc_id);
extern upk_packet_t    *upk_create_subscr_req(upk_protocol_handle_t * handle, char *svc_id, bool all_svcs);
extern upk_packet_t    *upk_create_unsubs_req(upk_protocol_handle_t * handle, char *svc_id, bool all_svcs);
extern upk_packet_t    *upk_create_discon_req(upk_protocol_handle_t * handle);


/* *******************************************************************************************************************
   convenience functions for replies, when its easier than building the structs yourself
   ****************************************************************************************************************** */
extern upk_packet_t    *upk_create_repl_preamble(uint32_t best_version);

extern upk_packet_t    *upk_create_repl_seq_start(upk_protocol_handle_t * handle, upk_msgtype_t seq_type,
                                                  uint32_t count);
extern upk_packet_t    *upk_create_repl_seq_end(upk_protocol_handle_t * handle, bool commit);

extern upk_packet_t    *upk_create_result_repl(upk_protocol_handle_t * handle, char *msg, bool successful);
extern upk_packet_t    *upk_create_listing_repl(upk_protocol_handle_t * handle, char *svc_id);
extern upk_packet_t    *upk_create_svcinfo_repl(upk_protocol_handle_t * handle, char *svc_id, upk_svcinfo_t * svcinfo);
extern upk_packet_t    *upk_create_ack_repl(upk_protocol_handle_t * handle);

/* FIXME: should probably use upk_error_t enum instead of char *errmsg */
extern upk_packet_t    *upk_create_error_repl(upk_protocol_handle_t * handle, char *svc_id, char *errmsg,
                                              upk_errlevel_t errlvl);


/* *******************************************************************************************************************
   convenience functions for pubmsg's, because, why not...
   ****************************************************************************************************************** */
extern upk_packet_t    *upk_create_pub_pubmsg(upk_protocol_handle_t * handle);
extern upk_packet_t    *upk_create_cancel_pubmsg(upk_protocol_handle_t * handle);


/* *******************************************************************************************************************
   housekeeping
   ****************************************************************************************************************** */
extern void             upk_pkt_free(upk_packet_t * pkt);

/* *******************************************************************************************************************
   packet encapsulation
   ****************************************************************************************************************** */
#endif

/**
  @}
  */
