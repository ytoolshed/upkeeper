#include <stdio.h>
#include "common/test.h"

int main (
          int ac,
          char **av 
          ) {
  printf("1..2\n");

  /* test */
  upk_test_is( 1, 1, "one is one" );
  /* test */
  upk_test_is( 2, 2, "two is two" );

  return(0);

}
