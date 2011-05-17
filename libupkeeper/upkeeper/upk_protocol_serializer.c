#include "protocol.h"
#include "v0_protocol.h"
#include "error.h"
#include "crc32.h"

/**********************************************************************************************************************
 **********************************************************************************************************************/
static                  upk_pkttype_t
upk_pkt_version_to_enum(uint32_t version_id)
{
    switch (version_id) {
    case 0:
        return PKT_V0_PROTO_LIMIT;
    }
    return 0;
}


/**********************************************************************************************************************
 **********************************************************************************************************************/
static void            *
upk_deserialize_payload(char *payload_buf, uint32_t version_id)
{
    switch (version_id) {
    case 0:
        return v0_deserialize_payload(payload_buf);
        break;
    }
    return NULL;
}

/**********************************************************************************************************************
 **********************************************************************************************************************/
static char            *
upk_serialize_payload(upk_packet_t * pkt)
{
    switch (pkt->version_id) {
    case 0:
        return v0_serialize_payload(pkt);
        break;
    }
    return NULL;
}

/**********************************************************************************************************************
 **********************************************************************************************************************/
upk_packet_t           *
upk_deserialize_packet(char *buf)
{
    upk_packet_t           *pkt = NULL;
    char                   *ptr = NULL;
    uint32_t                enum_buf = 0;                  /* XXX: Guarantee enums are in a uint32_t space; otherwise
                                                            * wierdness might ensue) */
    char                   *payload_buf = NULL;

    UPK_ERR_INIT;

    pkt = calloc(1, sizeof(*pkt));
    ptr = buf;

    memcpy(&pkt->version_id, buf, sizeof(pkt->version_id));
    ptr += sizeof(pkt->version_id);
    UPK_FUNC_ASSERT(pkt->version_id >= UPK_MIN_SUPPORTED_PROTO
                    && pkt->version_id <= UPK_MAX_SUPPORTED_PROTO, UPK_ERR_UNSUPP);

    memcpy(&enum_buf, ptr, sizeof(enum_buf));
    pkt->pkttype = (upk_pkttype_t) enum_buf;
    ptr += sizeof(enum_buf);
    UPK_FUNC_ASSERT(pkt->pkttype < upk_pkt_version_to_enum(pkt->version_id), UPK_ERR_INVALID_PKT);

    memcpy(&pkt->payload_len, ptr, sizeof(pkt->payload_len));
    ptr += sizeof(pkt->payload_len);
    UPK_FUNC_ASSERT(pkt->payload_len <= UPK_MAX_PACKET_SIZE, UPK_ERR_INVALID_PKT);

    payload_buf = calloc(1, pkt->payload_len);
    memcpy(payload_buf, ptr, pkt->payload_len);
    ptr += pkt->payload_len;

    memcpy(&pkt->crc32, ptr, sizeof(pkt->crc32));
    UPK_FUNC_ASSERT(upk_verify_crc32(payload_buf, pkt->crc32, pkt->payload_len), UPK_ERR_INVALID_PKT);

    pkt->payload = upk_deserialize_payload(payload_buf, pkt->version_id);

    IF_UPK_ERROR {
        if(pkt) {
            if(pkt->payload)
                free(pkt->payload);
            free(pkt);
        }
        if(payload_buf)
            free(payload_buf);
    }
    return pkt;
}

/**********************************************************************************************************************
 **********************************************************************************************************************/
char                   *
v0_serialize_packet(upk_packet_t * pkt)
{
    char                   *payload_buf;
    char                   *pkt_buf;
    char                   *ptr;

    pkt_buf = calloc(1, sizeof(*pkt) + pkt->payload_len - sizeof(size_t));
    ptr = pkt_buf;

    payload_buf = upk_serialize_payload(pkt);
    pkt->crc32 = upk_crc32(payload_buf, pkt->payload_len);

    memcpy(ptr, &pkt->version_id, sizeof(pkt->version_id));
    ptr += sizeof(pkt->version_id);

    memcpy(ptr, &pkt->pkttype, sizeof(pkt->pkttype));
    ptr += sizeof(pkt->pkttype);

    memcpy(ptr, &pkt->payload_len, sizeof(pkt->payload_len));
    ptr += sizeof(pkt->payload_len);

    memcpy(ptr, payload_buf, pkt->payload_len);
    ptr += sizeof(pkt->payload_len);

    memcpy(ptr, &pkt->crc32, sizeof(pkt->crc32));

    return pkt_buf;
}
