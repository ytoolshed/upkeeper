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

#define TESTCASE_NAME "uuid"
#define SUITE_NAME "uuid"

START_TEST(test_gen_uuid)
{
    upk_uuid_t foo = { 0 };
    char sbuf[36] = "";

    gen_uuid_bytes(&foo);
    uuid_to_string(sbuf, &foo); 
    printf("%s\n", sbuf);
}
END_TEST;

TCase           *
tcase(void)
{
    TCase                  *tcase = tcase_create(TESTCASE_NAME);

    tcase_add_test(tcase, foo);
    return tcase;
}

Suite           *
suite(void)
{
    Suite                  *suite = suite_create(SUITE_NAME);

    suite_add_tcase(suite, tcase());
    return suite;
}


int
main(void)
{
    int                     num_failed = 0;
    Suite                  *suite = foo_suite();
    SRunner                *runner = srunner_create(suite);

    srunner_run_all(runner, CK_VERBOSE);
    num_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
