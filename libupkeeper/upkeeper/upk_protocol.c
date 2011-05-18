#include "protocol.h"

/* *******************************************************************************************************************
 * upk_create_pkt: Create a packet
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_pkt(void *payload, uint32_t payload_len, upk_pkttype_t pkttype, uint32_t proto_ver)
{
    upk_packet_t           *pkt = NULL;

    pkt = calloc(1, sizeof(*pkt));
    pkt->version_id = proto_ver;
    pkt->pkttype = pkttype;
    pkt->payload_len = payload_len;
    pkt->payload = payload;
    pkt->crc32 = 0;                                        /* populated/utilized by serialize/deserialize; which assert 
                                                            * validity; everything else can assume packets are valid */

    return pkt;
}


/* *******************************************************************************************************************
 * req_preamble_len: size of a preamble payload
 * ****************************************************************************************************************** */
static                  uint32_t
req_preamble_len(upk_req_preamble_t * preamble)
{
    return (sizeof(*preamble) + preamble->client_name_len + 1);
}


/* *******************************************************************************************************************
 * upk_create_req_preamble: Create a preamble
 * ****************************************************************************************************************** */
upk_packet_t           *
upk_create_req_preamble(char *client_name)
{
    upk_req_preamble_t     *preamble = NULL;

    assert(preamble = calloc(1, sizeof(*preamble)));

    preamble->msgtype = REQ_PREAMBLE;
    preamble->min_supported_ver = UPK_MIN_SUPPORTED_PROTO;
    preamble->max_supported_ver = UPK_MAX_SUPPORTED_PROTO;
    preamble->client_name_len = strnlen(client_name, UPK_MAX_STRING_LEN);
    strncpy(preamble->client_name, client_name, preamble->client_name_len + 1);

    return upk_create_pkt(preamble, req_preamble_len(preamble), PKT_REQUEST, 0);
}
