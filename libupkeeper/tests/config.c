#include <check.h>
#include <upkeeper.h>

#define test_config_file test.conf

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


