/* Public domain. */

#include <sys/types.h>
#include <sys/wait.h>

int wait_nohang(wstat) int *wstat;
{
  return waitpid(-1,wstat,WNOHANG);
}
