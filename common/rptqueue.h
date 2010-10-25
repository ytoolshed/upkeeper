#ifndef RPTQUEUE_H
#define RPTQUEUE_H

/*
 * Readproctitle-esque queue
 */

struct rptqueue {
  /* a buffer to store recent logs, with length */
  char *display;
  int  dlen;
  /* our posisition in the buffer */
  char *dpos;
  /* a file descriptor to read from */
  int  ifd;
  /* a file descriptor to write to */
  int  ofd;
  /* our buffer */
  char buf[2];
};

/* returns the number of bytes transferred */
int rpt_status_update(struct rptqueue *rpt);
int rpt_write_status (struct rptqueue *rpt, const char msg);

void rpt_init(struct rptqueue *rpt);

#endif
