#include <check.h>
#include <upkeeper.h>

#define TESTCASE_NAME "util"
#define SUITE_NAME "util"

START_TEST(test_replace_larger)
{
    const char *repl = "Hello, this is a relatively arbitrarily sized string, thanks.";
    const char *needle = "%(SOME_STRING)";
    char *haystack = NULL; 
    
    haystack = calloc(1, strlen("#!/bin/bash\necho \"%(SOME_STRING)\"\necho bye") + 1);
    strcpy(haystack, "#!/bin/bash\necho \"%(SOME_STRING)\"\necho bye");

    upk_replace_string(&haystack, needle, repl);
    fail_unless(strcmp(haystack, "#!/bin/bash\necho \"Hello, this is a relatively arbitrarily sized string, thanks.\"\necho bye") == 0, "string was %s", haystack);

    free(haystack);
}
END_TEST;

START_TEST(test_replace_smaller)
{
    const char *repl = "Hello";
    const char *needle = "%(SOME_STRING)";
    char *haystack = NULL; 
    
    haystack = calloc(1, strlen("#!/bin/bash\necho \"%(SOME_STRING)\"\necho bye") + 1);
    strcpy(haystack, "#!/bin/bash\necho \"%(SOME_STRING)\"\necho bye");

    upk_replace_string(&haystack, needle, repl);
    fail_unless(strcmp(haystack, "#!/bin/bash\necho \"Hello\"\necho bye") == 0, "string was %s", haystack);

    free(haystack);
}
END_TEST;


TCase           *
test_tcase(void)
{
    TCase                  *tcase = tcase_create(TESTCASE_NAME);

    tcase_add_test(tcase, test_replace_larger);
    tcase_add_test(tcase, test_replace_smaller);
    return tcase;
}

Suite           *
test_suite(void)

{
    Suite                  *suite = suite_create(SUITE_NAME);

    suite_add_tcase(suite, test_tcase());
    return suite;
}


int
main(void)
{
    int                     num_failed = 0;
    Suite                  *suite = test_suite();
    SRunner                *runner = srunner_create(suite);

    srunner_run_all(runner, CK_VERBOSE);
    num_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
