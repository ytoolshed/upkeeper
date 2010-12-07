#include <fcntl.h>
#include <stdio.h>
#include "store/upk_db.h"

/* 
 * Let's test by doing a F_GETFL after a nonblock ?

 */ 
int main (void) {
  int pfd[2];
  int fl;
  printf("1..1\n");
  if (pipe(pfd) == -1) {
    perror("pipe");
  }
  fl = fcntl(pfd[1], F_GETFL, 0);
  upk_test_is(fl & O_NONBLOCK,0, "fd is blocking");
  nonblock(pfd[1]);
  upk_test_is(fl & O_NONBLOCK,0, "fd is non-block after nonblock()");

  
  return(0);
}
