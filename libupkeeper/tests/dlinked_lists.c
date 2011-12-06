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

typedef struct _test_foo test_foo_t;
struct _test_foo {
    int32_t                 val;
    int32_t                 anotherval;
    test_foo_t             *prev;
    test_foo_t             *next;
};

START_TEST(test_swap)
{
    int32_t n=0, cmp_val[] = { 4, 9, 10, 24, 243562 }, cmp_anotherval[] = { 5, 362, 239, -135, 5862 };
    uint32_t len = (sizeof(cmp_val)/sizeof(int32_t));
    test_foo_t *prev_a, *prev_b, *a, *b;
    int32_t buf;

    UPKDLIST_INIT(test_foo_t, foobar);

    for(n=0;n<len;n++) { 
        UPKDLIST_APPEND(foobar);
        foobar->thisp->val = cmp_val[n];
        foobar->thisp->anotherval = cmp_anotherval[n];
    }

    buf=cmp_val[1];
    cmp_val[1]=cmp_val[3];
    cmp_val[3]=buf;

    buf=cmp_anotherval[1];
    cmp_anotherval[1]=cmp_anotherval[3];
    cmp_anotherval[3]=buf;

    n=0;
    UPKDLIST_FOREACH(foobar) { 
        if(n == 1) { 
            prev_a = foobar->prevp;
            a = foobar->thisp;
        }
        else if(n == 3) {
            prev_b = foobar->prevp;
            b = foobar->thisp;
        }
        n++;
    }
    UPKDLIST_SWAP(foobar, a, b);

    n=0;
    UPKDLIST_FOREACH(foobar) {
        fail_unless(foobar->thisp->val == cmp_val[n], "val: %d, expected: %d", foobar->thisp->val, cmp_val[n]);
        fail_unless(foobar->thisp->anotherval == cmp_anotherval[n], "anotherval: %d, expected: %d", foobar->thisp->anotherval, cmp_anotherval[n]);
        ++n;
    }

    fail_unless(foobar->count == len, "foobar->count = %d, should be: %d", foobar->count, len);
    fail_unless(n == foobar->count, "n == %d", foobar->count);

    UPKDLIST_FREE(foobar);
    return;
}

END_TEST;



START_TEST(test_prepend)
{
    int32_t n=0, cmp_val[] = { 4, 9, 10, 24, 243562 }, cmp_anotherval[] = { 5, 362, 239, -135, 5862 };
    uint32_t len = (sizeof(cmp_val)/sizeof(int32_t));

    UPKDLIST_INIT(test_foo_t, foobar);

    for(n = 0; n < len; ++n) { 
        UPKDLIST_PREPEND(foobar);
        foobar->thisp->val = cmp_val[n]; 
        foobar->thisp->anotherval = cmp_anotherval[n];
    }

    n=len - 1;
    UPKDLIST_FOREACH(foobar) {
        fail_unless(foobar->thisp->val == cmp_val[n], "val = %d, cmp_val[%d] = %d", foobar->thisp->val, n, cmp_val[n]);
        fail_unless(foobar->thisp->anotherval == cmp_anotherval[n], "anotherval");
        --n;
    }

    fail_unless(foobar->count == len, "foobar->count");

    UPKDLIST_FREE(foobar);
    return;
}

END_TEST;



START_TEST(test_append)
{
    int32_t n=0, cmp_val[] = { 4, 9, 10, 24, 243562 }, cmp_anotherval[] = { 5, 362, 239, -135, 5862 };
    uint32_t len = (sizeof(cmp_val)/sizeof(int32_t));

    UPKDLIST_INIT(test_foo_t, foobar);

    for(n = 0; n < len; ++n) { 
        UPKDLIST_APPEND(foobar);
        foobar->thisp->val = cmp_val[n]; 
        foobar->thisp->anotherval = cmp_anotherval[n];
    }

    n=0;
    UPKDLIST_FOREACH(foobar) {
        fail_unless(foobar->thisp->val == cmp_val[n], "val");
        fail_unless(foobar->thisp->anotherval == cmp_anotherval[n], "anotherval");
        ++n;
    }

    fail_unless(foobar->count == len, "foobar->count");
    fail_unless(n == foobar->count, "n == %d", foobar->count);

    UPKDLIST_FREE(foobar);
    return;
}

END_TEST;



START_TEST(test_append_this)
{
    int32_t n=0, cmp_val[] = { 4, 9, 10, 24, 243562 }, cmp_anotherval[] = { 5, 362, 239, -135, 5862 };
    uint32_t len = (sizeof(cmp_val)/sizeof(int32_t));

    UPKDLIST_INIT(test_foo_t, foobar);

    for(n = 0; n < len; ++n) { 
        UPKDLIST_APPEND_THIS(foobar);
        foobar->thisp->val = cmp_val[n]; 
        foobar->thisp->anotherval = cmp_anotherval[n];
        foobar->prevp = foobar->thisp; 
    }

    n=0;
    UPKDLIST_FOREACH(foobar) {
        fail_unless(foobar->thisp->val == cmp_val[n], "val");
        fail_unless(foobar->thisp->anotherval == cmp_anotherval[n], "anotherval");
        ++n;
    }

    fail_unless(foobar->count == len, "foobar->count");
    fail_unless(n == foobar->count, "n == %d", foobar->count);

    UPKDLIST_FREE(foobar);
    return;
}

END_TEST;


START_TEST(test_unlink)
{
    int32_t n=0, cmp_val[] = { 4, 9, 10, 24, 243562 }, cmp_anotherval[] = { 5, 362, 239, -135, 5862 };
    uint32_t len = (sizeof(cmp_val)/sizeof(int32_t));

    UPKDLIST_INIT(test_foo_t, foobar);

    for(n = 0; n < len; ++n) { 
        UPKDLIST_APPEND_THIS(foobar);
        foobar->thisp->val = cmp_val[n];
        foobar->thisp->anotherval = cmp_anotherval[n];
        foobar->prevp = foobar->thisp;
    }

    n=0;
    UPKDLIST_FOREACH(foobar) { 
        if(n++ == 2) {
            UPKDLIST_UNLINK(foobar);
        }
    }

    for(n = 3; n < len; n++) {
        cmp_val[n-1]=cmp_val[n];
        cmp_anotherval[n-1]=cmp_anotherval[n];
    }

    n=0;
    UPKDLIST_FOREACH(foobar) {
        fail_unless(foobar->thisp->val == cmp_val[n], "val");
        fail_unless(foobar->thisp->anotherval == cmp_anotherval[n], "anotherval");
        ++n;
    }

    fail_unless(foobar->count == len - 1, "foobar->count");
    fail_unless(n == foobar->count, "n == %d", foobar->count);

    UPKDLIST_FREE(foobar);
    return;
}
END_TEST;

START_TEST(test_unlink_first)
{
    int32_t n=0, cmp_val[] = { 4, 9, 10, 24, 243562 }, cmp_anotherval[] = { 5, 362, 239, -135, 5862 };
    uint32_t len = (sizeof(cmp_val)/sizeof(int32_t));

    UPKDLIST_INIT(test_foo_t, foobar);

    for(n = 0; n < len; ++n) { 
        UPKDLIST_APPEND_THIS(foobar);
        foobar->thisp->val = cmp_val[n];
        foobar->thisp->anotherval = cmp_anotherval[n];
        foobar->prevp = foobar->thisp;
    }

    n=0;
    UPKDLIST_FOREACH(foobar) { 
        if(n++ == 0) {
            UPKDLIST_UNLINK(foobar);
        }
    }

    for(n = 1; n < len; n++) {
        cmp_val[n-1]=cmp_val[n];
        cmp_anotherval[n-1]=cmp_anotherval[n];
    }

    n=0;
    UPKDLIST_FOREACH(foobar) {
        fail_unless(foobar->thisp->val == cmp_val[n], "val");
        fail_unless(foobar->thisp->anotherval == cmp_anotherval[n], "anotherval");
        ++n;
    }

    fail_unless(foobar->count == len - 1, "foobar->count");
    fail_unless(n == foobar->count, "n == %d", foobar->count);

    UPKDLIST_FREE(foobar);
    return;
}
END_TEST;

START_TEST(reverse)
{
    int32_t n=0, cmp_val[] = { 4, 9, 10, 24, 243562 }, cmp_anotherval[] = { 5, 362, 239, -135, 5862 };
    uint32_t len = (sizeof(cmp_val)/sizeof(int32_t));

    UPKDLIST_INIT(test_foo_t, foobar);

    for(n = 0; n < len; ++n) { 
        UPKDLIST_APPEND_THIS(foobar);
        foobar->thisp->val = cmp_val[n];
        foobar->thisp->anotherval = cmp_anotherval[n];
        foobar->prevp = foobar->thisp;
    }

    n=len-1;
    UPKDLIST_FOREACH_REVERSE(foobar) {
        fail_unless(foobar->thisp->val == cmp_val[n], "val");
        fail_unless(foobar->thisp->anotherval == cmp_anotherval[n], "anotherval");
        --n;
    }

    fail_unless(foobar->count == len, "foobar->count: %d", foobar->count);
    fail_unless(n == -1, "n == %d", n);

    UPKDLIST_FREE(foobar);
    return;
}
END_TEST;





TCase           *
ll_tcase(void)
{
    TCase                  *ll = tcase_create("linked list macros");

    tcase_add_test(ll, test_swap);
    tcase_add_test(ll, test_prepend); 
    tcase_add_test(ll, test_append);
    tcase_add_test(ll, test_append_this);
    tcase_add_test(ll, test_unlink);
    tcase_add_test(ll, test_unlink_first);
    tcase_add_test(ll, reverse);
    return ll;
}

Suite           *
upk_ll_suite(void)
{
    Suite                  *suite = suite_create("linked lists");

    suite_add_tcase(suite, ll_tcase());
    return suite;
}


int
main(void)
{
    int                     num_failed = 0;
    Suite                  *suite = upk_ll_suite();
    SRunner                *runner = srunner_create(suite);

    srunner_run_all(runner, CK_VERBOSE);
    num_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
