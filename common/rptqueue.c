#include <string.h>
#include "rptqueue.h"

void rpt_init(struct rptqueue *rpt)
{
  rpt->buf[0] = -1;
  rpt->buf[1] = -1;
  rpt->dpos = rpt->display;

  if (rpt->ofd != -1)
    ndelay_on(rpt->ofd);

  ndelay_on(rpt->ifd);

  if (!rpt->dlen) {
    rpt->dlen = strlen(rpt->display);
  }
}

int rpt_status_update(struct rptqueue *rpt)
{
  int red;
  if (rpt->ifd == -1) return 1;

  while (read(rpt->ifd,&(rpt->buf[1]),1) > 0) {
    rpt_write_status(rpt,rpt->buf[1]);

    if (rpt->ofd != -1) 
      write(rpt->ofd,&rpt->buf[1],1);
  }

  return 1;
}
 
int rpt_write_status (struct rptqueue *rpt, const char msg)
{
  int i = 3;
  while (i++ < rpt->dlen) 
    rpt->display[i-1] = rpt->display[i];

  rpt->display[rpt->dlen-1] = msg;
  
}
