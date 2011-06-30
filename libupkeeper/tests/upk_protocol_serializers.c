#include <check.h>
#include <upkeeper/upk_protocol.h>

#define NAME_TO_FUNC(A) upk_create_ ## A
#define NAME_TO_TYPE(A) upk_ ## A ## _t

#define TEST_SETUP(NAME,...) \
    upk_packet_t *pkt = NULL; \
    NAME_TO_TYPE(NAME) t = { 0 }; \
    upk_protocol_handle_t handle = { 0 }; \
    pkt = NAME_TO_FUNC(NAME)(&handle,__VA_ARGS__); \
    upk_pkt_buf_t * buf = upk_serialize_packet(pkt); \
    upk_pkt_free(pkt); \
    pkt = upk_deserialize_packet(buf); \
    t = *((NAME_TO_TYPE(NAME) *) pkt->payload)

#define TEST_SETUP_NOARGS(NAME) \
    upk_packet_t *pkt = NULL; \
    NAME_TO_TYPE(NAME) t = { 0 }; \
    upk_protocol_handle_t handle = { 0 }; \
    pkt = NAME_TO_FUNC(NAME)(&handle); \
    upk_pkt_buf_t * buf = upk_serialize_packet(pkt); \
    upk_pkt_free(pkt); \
    pkt = upk_deserialize_packet(buf); \
    t = *((NAME_TO_TYPE(NAME) *) pkt->payload)

#define TEST_TEARDOWN \
    upk_pkt_free(pkt); \
    free(buf)

#define v0_svcinfo_t upk_svcinfo_t

#include "v0_protocol_generic_value_checks.inc.c"


Suite                  *
upk_proto_serializers_suite(void)
{
    Suite                  *suite = suite_create("upk_proto_serializers");

    suite_add_tcase(suite, v0_proto_requests());
    suite_add_tcase(suite, v0_proto_replies());
    suite_add_tcase(suite, v0_proto_pubmsgs());

    return suite;
}


int
main(void)
{
    int                     num_failed = 0;
    Suite                  *suite = upk_proto_serializers_suite();
    SRunner                *runner = srunner_create(suite);

    srunner_run_all(runner, CK_VERBOSE);
    num_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


