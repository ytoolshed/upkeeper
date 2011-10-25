
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
#include "upk_error.h"
#include "upk_crc32.h"

/* *******************************************************************************************************************
   XXX: Update these arrays when you add additional protocol versions
   ****************************************************************************************************************** */
upk_pkttype_t           upk_pkt_proto_limit[] = {
    PKT_V0_PROTO_LIMIT,
};
upk_msgtype_t           upk_req_proto_limit[] = {
    UPK_REQ_V0_PROTO_LIMIT,
};

upk_msgtype_t           upk_repl_proto_limit[] = {
    UPK_REPL_V0_PROTO_LIMIT,
};

upk_msgtype_t           upk_pub_proto_limit[] = {
    UPK_PUB_V0_PROTO_LIMIT,
};

#undef UPK_MSG_IDENTIFIER_TYPEDEF
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_msgtype_t

#undef UPK_MSG_IDENTIFIER
#define UPK_MSG_IDENTIFIER msgtype

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
void                   *
upk_deserialize_req_preamble(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(upk_req_preamble_t);

    UPK_FETCH_UINT32(min_supported_ver);
    UPK_FETCH_UINT32(max_supported_ver);
    UPK_FETCH_UINT32(client_name_len);
    UPK_FETCH_STRING(client_name);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_pkt_buf_t          *
upk_serialize_req_preamble(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(upk_req_preamble_t);

    UPK_PUT_UINT32(min_supported_ver);
    UPK_PUT_UINT32(max_supported_ver);
    UPK_PUT_UINT32(client_name_len);
    UPK_PUT_STRING(client_name);

    return UPK_BUF;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
void                   *
upk_deserialize_repl_preamble(upk_pkt_buf_t * UPK_BUF)
{
    UPK_INIT_DESERIALIZE_MSG(upk_repl_preamble_t);

    UPK_FETCH_UINT32(best_version);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_pkt_buf_t          *
upk_serialize_repl_preamble(void *UPK_DATA_PTR, size_t UPK_DATA_LEN)
{
    UPK_INIT_SERIALIZE_MSG(upk_repl_preamble_t);

    UPK_PUT_UINT32(best_version);

    return UPK_BUF;
}



/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static inline void     *
upk_deserialize_payload(upk_pkt_buf_t * payload_buf, upk_pkttype_t pkttype, uint32_t version_id)
{
    switch (version_id) {
    case 0:
        return v0_deserialize_payload(payload_buf, pkttype);
        break;
    }
    return NULL;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static inline upk_pkt_buf_t *
upk_serialize_payload(upk_packet_t * pkt)
{
    switch (pkt->version_id) {
    case 0:
        return v0_serialize_payload(pkt);
        break;
    }
    return NULL;
}

#undef UPK_MSG_IDENTIFIER_TYPEDEF
#define UPK_MSG_IDENTIFIER_TYPEDEF upk_pkttype_t

#undef UPK_MSG_IDENTIFIER
#define UPK_MSG_IDENTIFIER pkttype

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_packet_t           *
upk_deserialize_packet(upk_pkt_buf_t * UPK_BUF)
{
    upk_pkt_buf_t          *payload_buf = NULL;
    uint32_t                min_supported_proto = UPK_MIN_SUPPORTED_PROTO;

    UPK_ERR_INIT;
    UPK_INIT_DESERIALIZE(upk_packet_t);
    UPK_DATA = calloc(1, sizeof(*UPK_DATA));

    UPK_FETCH_UINT32(payload_len);
    UPK_FUNC_ASSERT(UPK_DATA->payload_len <= UPK_MAX_PACKET_SIZE, UPK_ERR_INVALID_PKT);

    UPK_FETCH_UINT32(version_id);
    UPK_FUNC_ASSERT(UPK_DATA->version_id >= min_supported_proto
                    && UPK_DATA->version_id <= UPK_MAX_SUPPORTED_PROTO, UPK_ERR_UNSUP);

    UPK_FETCH_UINT32(seq_num);

    UPK_FETCH_ENUM(upk_pkttype_t, pkttype);
    UPK_FUNC_ASSERT(UPK_DATA->pkttype < upk_pkt_proto_limit[UPK_DATA->version_id], UPK_ERR_INVALID_PKT);

    UPK_FETCH_DATA(payload);
    payload_buf = UPK_DATA->payload;
    UPK_DATA->payload = NULL;

    UPK_FETCH_UINT32(crc32);
    UPK_FUNC_ASSERT(upk_verify_crc32(payload_buf, UPK_DATA->crc32, UPK_DATA->payload_len), UPK_ERR_INVALID_PKT);

    UPK_DATA->payload = upk_deserialize_payload(payload_buf, UPK_DATA->pkttype, UPK_DATA->version_id);

    IF_UPK_ERROR {
        if(UPK_DATA) {
            if(UPK_DATA->payload)
                free(UPK_DATA->payload);
            free(UPK_DATA);
        }
    }
    if(payload_buf)
        free(payload_buf);

    return UPK_DATA;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
upk_pkt_buf_t          *
upk_serialize_packet(upk_packet_t * UPK_DATA_PTR)
{
    upk_pkt_buf_t          *payload_buf;

    UPK_INIT_SERIALIZE_BUF(upk_packet_t, sizeof(*UPK_DATA) + UPK_DATA->payload_len - sizeof(UPK_DATA->payload));

    payload_buf = upk_serialize_payload(UPK_DATA);
    UPK_DATA->crc32 = upk_crc32(payload_buf, UPK_DATA->payload_len);

    UPK_PUT_UINT32(payload_len);
    UPK_PUT_UINT32(version_id);
    UPK_PUT_UINT32(seq_num);
    UPK_PUT_ENUM(pkttype);
    UPK_PUT_DATA_FROM_BUF(payload_buf, payload_len);
    free(payload_buf);
    UPK_PUT_UINT32(crc32);

    return UPK_BUF;
}
