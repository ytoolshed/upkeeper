
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
#define UPK_PACKET_FOOTER_LEN 4

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
    /* memset(UPK_DATA->MEMB,0,sizeof(UPK_DATA->MEMB)); */ \
    /* UPK_DATA->MEMB = calloc(1, UPK_DATA->UPK_MEMB_TO_LEN(MEMB) + 1);  * null terminate */ \
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

/** *****************************************************************************************************************
  @brief packet type.

   enums for packet type description; anything >= V0_PROTO_LIMIT is invalid * in version 0 of the protocol; future
   protocol extensions may be added after V0_PROTO_LIMIT in enumeration, and terminated with V1_PROTO_LIMIT, etc. For
   the sake of simplicity; these will be forced into uint32_t space; regardless of what the arch's enum size may
   actually be
    ***************************************************************************************************************** */
typedef enum {
    PKT_REQUEST = 1,
    PKT_REPLY,
    PKT_PUBMSG,
    PKT_V0_PROTO_LIMIT,
} upk_pkttype_t;

/** *****************************************************************************************************************
  @brief message type.

 ** ***************************************************************************************************************** */
typedef enum _upk_msgtype {
    UPK_REQ_ORIGIN = 1,                                    /*!< idenitfy start of range */
    UPK_REQ_PREAMBLE = 1,                                  /*!< A preamble message, used to negotiate
                                                              version, and handshake */
    UPK_REQ_SEQ_START,                                     /*!< The start of a request sequence */
    UPK_REQ_SEQ_END,                                       /*!< the end of a request sequence */
    UPK_REQ_ACTION,                                        /*!< An action request (e.g. start, stop, reload, 
                                                              or custom actions) */
    UPK_REQ_SIGNAL,                                        /*!< a signal request (e.g. send signal N) */
    UPK_REQ_LIST,                                          /*!< A listing request. for a listing of all
                                                              service-ids known to the controller */
    UPK_REQ_STATUS,                                        /*!< A status request, to get information on a
                                                              particular service */
    UPK_REQ_SUBSCRIBE,                                     /*!< subscribe to a feed of status updates */
    UPK_REQ_UNSUBSCRIBE,                                   /*!< unsubscribe from a feed of status updates */
    UPK_REQ_DISCONNECT,                                    /*!< notify controller of your intent to
                                                              disconnect */
    UPK_REQ_V0_PROTO_LIMIT,                                /*!< all valid v0 requests are < this */
    UPK_REQ_LIMIT,                                         /*!< all requests are < this */
    UPK_REPL_ORIGIN = 4096,                                /*!< identify start of range */
    UPK_REPL_PREAMBLE = 4096,                              /*!< the reply to a preamble request, used to
                                                              send back what the server decided was the best
                                                              protocol, and complete the handshake */
    UPK_REPL_SEQ_START,                                    /*!< the start of a sequence of replies */
    UPK_REPL_SEQ_END,                                      /*!< the end of a sequence of replies */
    UPK_REPL_RESULT,                                       /*!< the result of an action or signal request
                                                              (or anything else that might benefit from a
                                                              result msg */
    UPK_REPL_LISTING,                                      /*!< the name of a service in a sequence listing
                                                              all services */
    UPK_REPL_SVCINFO,                                      /*!< all the information known about a service */
    UPK_REPL_ACK,                                          /*!< an ack, when nothing else really fits, but a 
                                                              reply is still mandated */
    UPK_REPL_ERROR,                                        /*!< report an error to the client, for instance, 
                                                              if the named service doesn't exist, or an
                                                              action doesn't exist, etc */
    UPK_REPL_V0_PROTO_LIMIT,                               /*!< All valid v0 replies are < this */
    UPK_REPL_LIMIT,                                        /*!< All replies are < this */
    UPK_PUB_ORIGIN = 8192,                                 /*!< pub origin */
    UPK_PUB_PUBLICATION = 8192,                            /*!< a message sent from the controller to a
                                                              subscriber, followed by svcinfo packets for all 
                                                              subscribed services */
    UPK_PUB_CANCELATION,                                   /*!< notification that a particular service is no 
                                                              longer available to subscribe to, for instance
                                                              if its been removed */
    UPK_PUB_V0_PROTO_LIMIT,                                /*!< All valid pub v0 publication messages are <
                                                              this */
    UPK_PUB_LIMIT,                                         /*!< All publication messages are < this */
} upk_msgtype_t;

#define UPK_N_SUPPORTED_PROTOS (1 + UPK_MAX_SUPPORTED_PROTO - UPK_MIN_SUPPORTED_PROTO)
#define _UPK_N_PER_TYPE(A) (UPK_##A##_LIMIT - UPK_##A##_ORIGIN - UPK_N_SUPPORTED_PROTOS)
#define UPK_N_REQ_TYPES _UPK_N_PER_TYPE(REQ)
#define UPK_N_REPL_TYPES _UPK_N_PER_TYPE(REPL)
#define UPK_N_PUB_TYPES _UPK_N_PER_TYPE(PUB)

#define _UPK_MSGTYPE_OFFSET(V,A,B) (V < UPK_##A##_LIMIT) ? (V - UPK_##A##_ORIGIN) + B
#define UPK_MSGTYPE_IDX(MSGTYPE) \
    ( _UPK_MSGTYPE_OFFSET(MSGTYPE,REQ,0) : _UPK_MSGTYPE_OFFSET(MSGTYPE,REPL,UPK_N_REQ_TYPES) : \
    _UPK_MSGTYPE_OFFSET(MSGTYPE,PUB,UPK_N_REQ_TYPES+UPK_N_REPL_TYPES) : -1 )

/** *******************************************************************************************************************
   |----|----|----|----|<payload ...>|----|
   |<----  header  --->|             |foot|
   ****************************************************************************************************************** */
typedef struct {
    uint32_t                payload_len;                   /*!< not size_t, because this would force the
                                                              arch of the client to match the server; and
                                                              even on localhost, that cannot be guaranteed */
    uint32_t                version_id;                    /*!< this packet's version */
    uint32_t                seq_num;                       /*!< not used at this time */
    upk_pkttype_t           pkttype;                       /*!< will be forced into a uint32_t; even on
                                                              64bit */
    void                   *payload;                       /*!< the payload */
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
    char                    client_name[UPK_MAX_STRING_LEN];
} upk_req_preamble_t;


/* *******************************************************************************************************************
   |----|----|
   ****************************************************************************************************************** */
typedef struct {
    upk_msgtype_t           msgtype;
    uint32_t                best_version;
} upk_repl_preamble_t;

typedef struct {
    upk_msgtype_t           msgtype;
} upk_generic_msg_t;

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
    UPK_V0_REQ_ACTION_T_FIELDS;
} upk_req_action_t;

typedef struct {
    UPK_V0_REQ_SIGNAL_T_FIELDS;
} upk_req_signal_t;

typedef struct {
    UPK_V0_REQ_LIST_T_FIELDS;
} upk_req_list_t;

typedef struct {
    UPK_V0_REQ_STATUS_T_FIELDS;
} upk_req_status_t;

typedef struct {
    UPK_V0_REQ_SUBSCR_T_FIELDS;
} upk_req_subscribe_t;

typedef struct {
    UPK_V0_REQ_UNSUBS_T_FIELDS;
} upk_req_unsubscribe_t;

typedef struct {
    UPK_V0_REQ_DISCON_T_FIELDS;
} upk_req_disconnect_t;

typedef struct {
    UPK_V0_UPK_REPL_SEQ_START_T_FIELDS;
} upk_repl_seq_start_t;

typedef struct {
    UPK_V0_UPK_REPL_SEQ_END_T_FIELDS;
} upk_repl_seq_end_t;

typedef struct {
    UPK_V0_REPL_RESULT_T_FIELDS;
} upk_repl_result_t;

typedef struct {
    UPK_V0_REPL_LISTING_T_FIELDS;
} upk_repl_listing_t;

/* Moved definition to types.h */
/* typedef struct { UPK_V0_SVCINFO_T_FIELDS; } upk_svcinfo_t; */

/* redefine v0_svcinfo_t with upk_svcinfo_t so we can borrow the struct definition while still referencing
   the correct, version-agnostic, structure therein */
#define v0_svcinfo_t upk_svcinfo_t
typedef struct {
    UPK_V0_REPL_SVCINFO_T_FIELDS;
} upk_repl_svcinfo_t;

#undef v0_svcinfo_t

typedef struct {
    UPK_V0_REPL_ACK_T_FIELDS;
} upk_repl_ack_t;

typedef struct {
    UPK_V0_REPL_ERROR_T_FIELDS;
} upk_repl_error_t;

typedef struct {
    UPK_V0_PUBLICATION_T_FIELDS;
} upk_pub_publication_t;

typedef struct {
    UPK_V0_CANCELATION_T_FIELDS;
} upk_pub_cancelation_t;

typedef union _upk_payload_types {
    upk_req_preamble_t      req_preamble;
    upk_repl_preamble_t     repl_preamble;
    upk_req_seq_start_t     req_seq_start;
    upk_req_seq_end_t       req_seq_end;
    upk_req_action_t        req_action;
    upk_req_signal_t        req_signal;
    upk_req_list_t          req_list;
    upk_req_status_t        req_status;
    upk_req_subscribe_t     req_subscribe;
    upk_req_unsubscribe_t   req_unsubscribe;
    upk_req_disconnect_t    req_disconnect;
    upk_repl_seq_start_t    repl_seq_start;
    upk_repl_seq_end_t      repl_seq_end;
    upk_repl_result_t       repl_result;
    upk_repl_listing_t      repl_listing;
    upk_repl_svcinfo_t      repl_svcinfo;
    upk_repl_ack_t          repl_ack;
    upk_repl_error_t        repl_error;
    upk_pub_publication_t   pub_publication;
    upk_pub_cancelation_t   pub_cancelation;
} upk_payload_types_u;

typedef struct _upk_payload {
    upk_msgtype_t           type;
    upk_payload_types_u     payload;
} upk_payload_t;

#include "upk_network_types.h"
typedef upk_conn_handle_t upk_protocol_handle_t;



/* *******************************************************************************************************************
   function prototypes for serializing, deserializing, creating, and otherwise manipulating packets.
   ****************************************************************************************************************** */
extern upk_pkt_buf_t   *upk_serialize_packet(upk_packet_t * UPK_DATA_PTR);
extern upk_packet_t    *upk_deserialize_packet(upk_pkt_buf_t * UPK_BUF);

extern void            *upk_deserialize_req_preamble(upk_pkt_buf_t * UPK_BUF);
extern upk_pkt_buf_t   *upk_serialize_req_preamble(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);
extern void            *upk_deserialize_repl_preamble(upk_pkt_buf_t * UPK_BUF);
extern upk_pkt_buf_t   *upk_serialize_repl_preamble(void *UPK_DATA_PTR, size_t UPK_DATA_LEN);

extern upk_packet_t    *upk_create_pkt(void *payload, uint32_t payload_len, upk_pkttype_t pkttype,
                                       uint32_t proto_ver);

/* *******************************************************************************************************************
   convenience functions for requests, when its easier than building the structs yourself
   ****************************************************************************************************************** */
extern upk_packet_t    *upk_create_req_preamble(upk_protocol_handle_t * handle, char *client_name);

extern upk_packet_t    *upk_create_req_seq_start(upk_protocol_handle_t * handle, upk_msgtype_t seq_type,
                                                 uint32_t count);
extern upk_packet_t    *upk_create_req_seq_end(upk_protocol_handle_t * handle, bool commit);

extern upk_packet_t    *upk_create_req_action(upk_protocol_handle_t * handle, char *svc_id, char *action);
extern upk_packet_t    *upk_create_req_signal(upk_protocol_handle_t * handle, char *svc_id,
                                              upk_signal_t signal, bool signal_sid, bool signal_pgrp);
extern upk_packet_t    *upk_create_req_list(upk_protocol_handle_t * handle);
extern upk_packet_t    *upk_create_req_status(upk_protocol_handle_t * handle, char *svc_id);
extern upk_packet_t    *upk_create_req_subscribe(upk_protocol_handle_t * handle, char *svc_id, bool all_svcs);
extern upk_packet_t    *upk_create_req_unsubscribe(upk_protocol_handle_t * handle, char *svc_id,
                                                   bool all_svcs);
extern upk_packet_t    *upk_create_req_disconnect(upk_protocol_handle_t * handle);


/* *******************************************************************************************************************
   convenience functions for replies, when its easier than building the structs yourself
   ****************************************************************************************************************** */
extern upk_packet_t    *upk_create_repl_preamble(upk_protocol_handle_t * handle, uint32_t best_version);

extern upk_packet_t    *upk_create_repl_seq_start(upk_protocol_handle_t * handle, upk_msgtype_t seq_type,
                                                  uint32_t count);
extern upk_packet_t    *upk_create_repl_seq_end(upk_protocol_handle_t * handle, bool commit);

extern upk_packet_t    *upk_create_repl_result(upk_protocol_handle_t * handle, char *msg, bool successful);
extern upk_packet_t    *upk_create_repl_listing(upk_protocol_handle_t * handle, char *svc_id);
extern upk_packet_t    *upk_create_repl_svcinfo(upk_protocol_handle_t * handle, char *svc_id,
                                                upk_svcinfo_t * svcinfo);
extern upk_packet_t    *upk_create_repl_ack(upk_protocol_handle_t * handle);

/* FIXME: should probably use upk_error_t enum instead of char *errmsg */
extern upk_packet_t    *upk_create_repl_error(upk_protocol_handle_t * handle, char *svc_id,
                                              upk_errno_t uerrno, char *errmsg, upk_errlevel_t errlvl);


/* *******************************************************************************************************************
   convenience functions for pubmsg's, because, why not...
   ****************************************************************************************************************** */
extern upk_packet_t    *upk_create_pub_publication(upk_protocol_handle_t * handle);
extern upk_packet_t    *upk_create_pub_cancelation(upk_protocol_handle_t * handle);


/* *******************************************************************************************************************
   housekeeping
   ****************************************************************************************************************** */
extern void             upk_pkt_free(upk_packet_t * pkt);

/* *******************************************************************************************************************
   msgtype utilities
   ****************************************************************************************************************** */
extern upk_msgtype_t    upk_get_msgtype(upk_packet_t * pkt);
extern size_t           upk_get_msgsize(upk_msgtype_t type);

/* *******************************************************************************************************************
   packet encapsulation
   ****************************************************************************************************************** */
#endif

/**
  @}
  */
