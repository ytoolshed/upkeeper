#include <unistd.h>
#include <fcntl.h>

int coe (int fd) 
{
  return fcntl(fd,F_SETFD,FD_CLOEXEC);
}
