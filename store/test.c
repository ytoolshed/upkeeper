#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#include "upk_db.h"

extern int DEBUG;

int TESTS = 0;

int upk_test_is(
    int is,
    int should
) {
    if(is == should) {
        printf("ok %d\n", ++TESTS);
    } else {
        printf("not ok %d\n", ++TESTS);
        printf(" should be '%d' but is '%d'\n", should, is);
    }

    return( 0 );
}

int upk_test_eq(
    const char *is,
    const char *should
) {

    if(is == should) {
        printf("ok %d\n", ++TESTS);
        return( 1 );
    }

    if( is == NULL ) {
        printf("not ok %d\n", ++TESTS);
        return( 0 );
    }

    if( strcmp( is, should ) == 0 ) {
        printf("ok %d\n", ++TESTS);
        return( 1 );
    }

    printf("not ok %d\n", ++TESTS);
    printf(" should be '%s' but is '%s'\n", should, is);
    return( 0 );
}
