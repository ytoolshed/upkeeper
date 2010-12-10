#ifndef TEST_H
#define TEST_H


int upk_test_is(
    int is,
    int should,
    const char *msg
);

int upk_test_eq(
    const char *is,
    const char *should
);
int upk_test_isnt(
    int is,
    int shouldnt ,
    const char *msg
);


#endif
