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
