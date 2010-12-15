#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/uio.h>

void syswarn3(char *a, char *b, char *c)
{
  char *err = strerror(errno);
  struct iovec i[6];
  i[0].iov_base = a; i[0].iov_len = strlen(a);
  i[1].iov_base = b; i[1].iov_len = strlen(b);
  i[2].iov_base = c; i[2].iov_len = strlen(c);
  i[3].iov_base = ": "; i[3].iov_len = 2;
  i[4].iov_base = err;  i[4].iov_len = strlen(err);
  i[5].iov_base = "\n"; i[5].iov_len = 1;
  if (writev(2, i, 6));
}

void sysdie2(int e, char *a, char *b)
{
  syswarn3(a,b,"");
  _exit(e);
}

void sysdie3(int e, char *a, char *b, char *c)
{
  syswarn3(a,b,c);
  _exit(e);
}
