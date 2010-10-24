#include "rptqueue.h"

void rpt_init(struct rptqueue *rpt)
{
  rpt->buf = { -1, '\0' };
  rpt->dpos = display;

  if (rpt->ofd != -1)
    ndelay_on(rpt->ofd);

  ndelay_on(rpt->ifd);

  if (!rpt->dlen) {
    rpt->dlen = strlen(display);
  }
}

static void dooutput (struct rptqueue *rpt) 
{
  if (rpt->ofd != -1) {}

  
  rpt->buf[1] = '\0';
  return;

}  

int rpt_status_update(struct rptqueue *rpt)
{
  int read;
  dooutput(rpt);
  if (rpt->ifd == -1) return;

  red = read(rpt->ifd,buf[1],1);
  if (red != 1) return red;

  rpt_write_status(rpt,rpt->buf, 1);

  return 1;
}
 
int rpt_write_status (struct rptqueue *rpt, const char *msg, int len)
{
  int tostore = len, tomove;
  while((dpos < rpt->display + rpt->dlen) && tostore--) {
    *dpos++ = *msg++;
  }

  tomove = rpt->dlen - len;
  if (tomove > 0) {
    byte_copy(rpt->display + 3, tomove, rpt->display + 3 + tomove);
    byte_copy(rpt->display + 3 + tomove, len, msg);
  } else {
    byte_copy(rpt->display + 3, rpt->dlen - 3, msg + len - rpt->dlen - 3);
  }

}
