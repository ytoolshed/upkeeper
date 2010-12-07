#include <signal.h>
#include <stdio.h>
#include "sigblock.h"
/* This guy should be fun.
 * set up some signal handlers, send some signals to ourselves!
 */

int child = 0;
int term  = 0;

void termhandler(int ign) {
  term++;
}
void childhandler(int ign) {
  child++;
}

int main (void) {
  printf("1..7\n");

  upk_catch_signal(SIGTERM,termhandler);
  upk_catch_signal(SIGCHLD,childhandler);


  kill(getpid(), SIGTERM);
  kill(getpid(), SIGCHLD);
  upk_test_is(term,1,"got a sigterm");
  upk_test_is(child,1,"got a sigchild");

  upk_block_signal(SIGTERM);
  upk_block_signal(SIGCHLD);
  kill(getpid(), SIGTERM);
  kill(getpid(), SIGCHLD);
  upk_test_is(term,1, "blocked a sigterm");
  upk_test_is(child,1,"blocked a sigchild");


  upk_unblock_signal(SIGTERM);
  upk_unblock_signal(SIGCHLD);
  upk_test_is(term,2, "sigterm delivered");
  upk_test_is(child,2,"sigchild delivered");


  upk_uncatch_signal(SIGCHLD);
  kill(getpid(), SIGCHLD);
  upk_test_is(child,2,"uncaught sigchild");

  return 0;
}
