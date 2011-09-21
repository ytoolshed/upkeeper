/* ***************************************************************************
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

#include <check.h>
#include <upkeeper/upk_protocol.h>

#define NAME_TO_FUNC(A) upk_create_ ## A
#define NAME_TO_TYPE(A) upk_ ## A ## _t

#define TEST_SETUP(NAME,...) \
    upk_packet_t *pkt = NULL; \
    NAME_TO_TYPE(NAME) t = { 0 }; \
    upk_protocol_handle_t handle = { 0 }; \
    pkt = NAME_TO_FUNC(NAME)(&handle, __VA_ARGS__); \
    t = *((NAME_TO_TYPE(NAME) *) pkt->payload)

#define TEST_SETUP_NOARGS(NAME) \
    upk_packet_t *pkt = NULL; \
    NAME_TO_TYPE(NAME) t = { 0 }; \
    upk_protocol_handle_t handle = { 0 }; \
    pkt = NAME_TO_FUNC(NAME)(&handle); \
    t = *((NAME_TO_TYPE(NAME) *) pkt->payload)

#define TEST_TEARDOWN \
    upk_pkt_free(pkt);

#define v0_svcinfo_t upk_svcinfo_t
#define CHECK_PREAMBLE

#include "v0_protocol_generic_value_checks.inc.c"

Suite                  *
upk_proto_helpers_suite(void)
{
    Suite                  *suite = suite_create("upk_proto_helpers");

    suite_add_tcase(suite, v0_proto_requests());
    suite_add_tcase(suite, v0_proto_replies());
    suite_add_tcase(suite, v0_proto_pubmsgs());

    return suite;
}


int
main(void)
{
    int                     num_failed = 0;
    Suite                  *suite = upk_proto_helpers_suite();
    SRunner                *runner = srunner_create(suite);

    srunner_run_all(runner, CK_VERBOSE);
    num_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


