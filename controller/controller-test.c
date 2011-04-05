#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "common/test.h"
#include "controller.h"

int DEBUG = 0;
int main (
          int ac,
          char **av 
          ) {
  printf("1..2\n");
  chdir("controller");
  struct upk_db db;
  int rc = upk_db_init( &db );
  upk_test_is(rc, 0, "db init succeeded");

  if(rc < 0) {
    printf("upk_db_init failed. Exiting.\n");
    exit(-1);
  }

  upkctl_t ctl = upk_controller_init(&db);
  upk_test_isnt(ctl,0,"initialized controller");

  
  upk_controller_free(ctl);
  return(0);

}
