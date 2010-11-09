#include <unistd.h>
#include <fcntl.h>

void nonblock(int fd) 
{
  fcntl(fd, F_SETFL, O_NONBLOCK);
}
