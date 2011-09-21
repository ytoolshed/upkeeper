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
#include <upkeeper.h>
#include <stdio.h>

#define TESTCASE_NAME "uuid"
#define SUITE_NAME "uuid"

START_TEST(test_string_to_uuid)
{
    char buf[37] = "12345678-1234-5678-9ABC-0123456789AB";
    upk_uuid_t foo = { 0 };

    upk_string_to_uuid(buf, &foo);

    fail_unless(foo.time_low == 0x12345678, "time_low: %02X", foo.time_low);
    fail_unless(foo.time_mid == 0x1234, "time_mid: %02X", foo.time_mid);
    fail_unless(foo.time_high_and_version == 0x5678, "time_high_and_version: %02X", foo.time_high_and_version);
    fail_unless(foo.clk_seq_high == 0x9A, "clk_seq_high: %02X", foo.clk_seq_high);
    fail_unless(foo.clk_seq_low == 0xBC, ".clk_seq_low: %02X", foo.clk_seq_low);
    fail_unless(foo.node[0] == 0x01, "node[0]: %02X", foo.node[0]);
    fail_unless(foo.node[1] == 0x23, "node[1]: %02X", foo.node[1]);
    fail_unless(foo.node[2] == 0x45, "node[2]: %02X", foo.node[2]);
    fail_unless(foo.node[3] == 0x67, "node[3]: %02X", foo.node[3]);
    fail_unless(foo.node[4] == 0x89, "node[4]: %02X", foo.node[4]);
    fail_unless(foo.node[5] == 0xAB, "node[5]: %02X", foo.node[5]);
}
END_TEST;


START_TEST(test_uuid_to_string)
{
    char buf[37] = "";

    upk_uuid_t              foo = {
        .time_low = 0x12345678,
        .time_mid = 0x1234,
        .time_high_and_version = 0x5678,
        .clk_seq_high = 0x9A,
        .clk_seq_low = 0xBC,
        .node = {
                  0x01,
                  0x23,
                  0x45,
                  0x67,
                  0x89,
                  0xAB}
    };

    upk_uuid_to_string(buf, &foo);
    fail_unless(strcmp(buf, "12345678-1234-5678-9ABC-0123456789AB") == 0, "string is: %s, should be 12345678-1234-5678-9ABC-0123456789AB");
}

END_TEST;


START_TEST(test_is_valid)
{
    fail_unless(is_valid_upk_uuid_string("12345678-1234-1234-1234-123456789ABC"),
                "is_valid_upk_uuid_string identifies a valid string");
    fail_unless(is_valid_upk_uuid_string("abcdef78-1234-1234-1234-123456789ABC"),
                "is_valid_upk_uuid_string identifies an invalid string");
    fail_unless(!is_valid_upk_uuid_string("12345678:1234:1234:1234:123456789ABC"),
                "is_valid_upk_uuid_string identifies an invalid string");
    fail_unless(!is_valid_upk_uuid_string("12345678-1234-1234-1234-123456789AB"),
                "is_valid_upk_uuid_string identifies an invalid string");
    fail_unless(!is_valid_upk_uuid_string("12345678-1234-1234-1234-123456789ABCD"),
                "is_valid_upk_uuid_string identifies an invalid string");
    fail_unless(!is_valid_upk_uuid_string("a2345678-1234-1234-1234-123456789ABCG"),
                "is_valid_upk_uuid_string identifies an invalid string");
}

END_TEST;

START_TEST(test_gen_uuid)
{
    upk_uuid_t              foo = { 0 };

    upk_gen_uuid_bytes(&foo);

    fail_unless((foo.time_high_and_version & 0xF000) == 0x4000, "version should be 0x4000, is: %04X",
                foo.time_high_and_version);
    fail_unless(((foo.clk_seq_high & 0xF0) >= 0x80)
                && ((foo.clk_seq_high & 0xF0) <= 0xB0), "clk_seq_high should be 0x80, 0x90, 0xA0, or 0xB0; is: %02X",
                foo.clk_seq_high);
}

END_TEST;

TCase                  *
upk_tcase(void)
{
    TCase                  *tcase = tcase_create(TESTCASE_NAME);

    tcase_add_test(tcase, test_string_to_uuid);
    tcase_add_test(tcase, test_uuid_to_string);
    tcase_add_test(tcase, test_is_valid);
    tcase_add_test(tcase, test_gen_uuid);
    return tcase;
}

Suite                  *
upk_suite(void)
{
    Suite                  *suite = suite_create(SUITE_NAME);

    suite_add_tcase(suite, upk_tcase());
    return suite;
}


int
main(void)
{
    int                     num_failed = 0;
    Suite                  *suite = upk_suite();
    SRunner                *runner = srunner_create(suite);

    srunner_run_all(runner, CK_VERBOSE);
    num_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
