#include "rptqueue.h"
#include "test.h"
#include <unistd.h>

static char display[24] = ".......................";
int main (void) 
{
  int pipeto[2]; int pipefrom[2];
  struct rptqueue rpt;
  char buf[5];
  rpt.display = display;
  rpt.dlen    = 0;

  if (pipe(pipeto)) {
    perror("pipe");
  }
  if (pipe(pipefrom)) {
    perror("pipe");
  }
  rpt.ifd = pipeto[0];
  rpt.ofd = pipefrom[1];

  rpt_init(&rpt);
  upk_test_is(rpt.dlen, 23, "right length for display");
  rpt_write_status(&rpt,"abcdefghijklmnop");
  upk_test_eq(display,".......abcdefghijklmnop");
  write(pipeto[1],"a",1);
  rpt_status_update(&rpt);
  upk_test_eq(display,"......abcdefghijklmnopa");
  read(pipefrom[0],buf,1);
  upk_test_is(buf[0],'a',"passthrough works");
  rpt_write_status(&rpt,"abcdefghijklmnop");
  upk_test_eq(display,"...nopaabcdefghijklmnop");
  return 0;
}
