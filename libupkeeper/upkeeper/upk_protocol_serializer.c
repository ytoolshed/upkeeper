#include "protocol.h"
#include "v0_protocol.h"
#include "error.h"
#include "crc32.h"

/* *******************************************************************************************************************
 * XXX: Update these arrays when you add additional protocol versions
 * ****************************************************************************************************************** */
upk_pkttype_t           upk_pkt_proto_limit[] = {
    PKT_V0_PROTO_LIMIT,
};
upk_req_msgtype_t       upk_req_proto_limit[] = {
    REQ_V0_PROTO_LIMIT,
};

upk_repl_msgtype_t      upk_repl_proto_limit[] = {
    REPL_V0_PROTO_LIMIT,
};

upk_pub_msgtype_t       upk_pub_proto_limit[] = {
    PUB_V0_PROTO_LIMIT,
};

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static void            *
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
 * ****************************************************************************************************************** */
static upk_pkt_buf_t   *
upk_serialize_payload(upk_packet_t * pkt)
{
    switch (pkt->version_id) {
    case 0:
        return v0_serialize_payload(pkt);
        break;
    }
    return NULL;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_deserialize_packet(upk_pkt_buf_t * UPK_BUF)
{
    upk_pkt_buf_t          *payload_buf = NULL;

    UPK_ERR_INIT;
    UPK_INIT_DESERIALIZE_MSG(upk_packet_t);

    UPK_FETCH_SCALAR(version_id);
    UPK_FUNC_ASSERT(UPK_DATA->version_id >= UPK_MIN_SUPPORTED_PROTO
                    && UPK_DATA->version_id <= UPK_MAX_SUPPORTED_PROTO, UPK_ERR_UNSUP);

    UPK_FETCH_ENUM(upk_pkttype_t, pkttype);
    UPK_FUNC_ASSERT(UPK_DATA->pkttype < upk_pkt_proto_limit[UPK_DATA->version_id], UPK_ERR_INVALID_PKT);

    UPK_FETCH_SCALAR(payload_len);
    UPK_FUNC_ASSERT(UPK_DATA->payload_len <= UPK_MAX_PACKET_SIZE, UPK_ERR_INVALID_PKT);

    UPK_FETCH_DATA(payload, payload_len);
    payload_buf = UPK_DATA->payload;

    UPK_FETCH_SCALAR(crc32);
    UPK_FUNC_ASSERT(upk_verify_crc32(UPK_DATA->payload, UPK_DATA->crc32, UPK_DATA->payload_len), UPK_ERR_INVALID_PKT);

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
 * ****************************************************************************************************************** */
upk_pkt_buf_t          *
v0_serialize_packet(upk_packet_t * UPK_DATA_PTR)
{
    upk_pkt_buf_t          *payload_buf;

    UPK_INIT_SERIALIZE_BUF(upk_packet_t, sizeof(*UPK_DATA) + UPK_DATA->payload_len - sizeof(size_t));

    payload_buf = upk_serialize_payload(UPK_DATA);
    UPK_DATA->crc32 = upk_crc32(payload_buf, UPK_DATA->payload_len);

    UPK_PUT_SCALAR(version_id);
    UPK_PUT_ENUM(pkttype);
    UPK_PUT_SCALAR(payload_len);
    UPK_PUT_DATA_FROM_BUF(payload_buf, payload_len);
    UPK_PUT_SCALAR(crc32);

    return UPK_BUF;
}
