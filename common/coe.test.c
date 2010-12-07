#include <stdio.h>
#include <unistd.h>
#include "coe.h"
#include <errno.h>

int main(int ac, char **argv) {
  char *args[3] = { "dooo", "lalala", 0 };
  int pfd[2];
  int ret; 
  int serno;

  if (ac == 2) {
    ret = write(30," ",1);
    serno = errno;
    upk_test_is(ret,      -1, "write to the pipe failed");
    upk_test_is(serno, EBADF, "because it's now a bad file descriptor");
    ret = write(40," ",1);
    upk_test_is(ret,      1, "write to the non-coe pipe succeeded");
    return 0;

  }
  printf("1..3\n");     
  fflush(stdout);
  if (pipe(pfd) == -1) {
    perror("pipe");
  }

  if (dup2(pfd[1], 30) == -1) {
    perror("dup2");
  }
  if (dup2(pfd[1], 40) == -1) {
    perror("dup2");
  }
  coe(30);
  nonblock(pfd[1]);
  switch(fork()) {
  case -1:
    perror("fork");
    break;
  case 0: /* child */
    execve(argv[0], args, NULL);
  default: /* parent */
    wait(NULL);
  }
  return 0;
  

}
