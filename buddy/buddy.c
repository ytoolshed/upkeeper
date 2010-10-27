#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <getopt.h>
#include "common/sig.h"
#include "common/iopause.h"
#include "common/tai.h"
#include "common/taia.h"
#include "common/rptqueue.h"
#include "common/error.h"
#include "common/wait.h"
#include "common/strerr.h"
#include "common/sig.h"

#define FATAL "buddy: fatal: "

static const char *djb_compat_dir = NULL;
static       char *lbuf = NULL;

static int parentfd = 3;                         
static int logpipe[2] = { -1, -1 },
  selfpipe[2];

int child = 0, term = 0;
int pid;

static void idle (void);
static void handler(int sig)  
{ 
  if (sig == sig_child) child = 1;
  else term = 1;
  sig = write(selfpipe[1]," ",1); 
} 


int main(int ac, char **av, char **envp) 
{
  int i;
  char *prog = av[options_parse(ac,av,envp)];

  if (parentfd) {
    coe(parentfd);
    ndelay_on(parentfd);
  }

  sig_catch(sig_child, handler);
  sig_catch(sig_term, handler);
  sig_block(sig_child);
  
  if (lbuf) {
    if (pipe(logpipe) == -1) 
      strerr_die3sys(111,FATAL, "failed to create log pipe for ",prog);

    ndelay_on(logpipe[0]); ndelay_on(logpipe[1]);
    coe(logpipe[0]); coe(logpipe[1]);
  }

  if (pipe(selfpipe) == -1)
    strerr_die3sys(111,FATAL, "failed to create pipe for ",prog);

  coe(selfpipe[0]); coe(selfpipe[1]);
  ndelay_on(selfpipe[0]);

  if ((pid = fork()) == -1) 
    strerr_die3sys(111,FATAL, "failed to create pipe for ",prog);
  

  if (pid == 0) {
    sig_uncatch(sig_child);
    sig_uncatch(sig_term);
    sig_unblock(sig_child);

    if (lbuf) {
      if (fd_copy(1, logpipe[1]) == -1)
        strerr_die3sys(111,FATAL, "failed to set log for ",prog);
      
      close(logpipe[1]);
      close(logpipe[0]);
    }
    execle(prog, prog, NULL, envp);
    
    strerr_die3sys(111,FATAL, "failed to exec ",prog);
  }

  if (lbuf) 
    close(logpipe[1]);
  
  (void)write(parentfd, &pid, 4);
    
  idle();
}

static void idle (void)
{

  struct taia now, deadline;
  iopause_fd x[2] = {};
  char sig;
  int status, i, ret,wpid;
  int limit = lbuf ? 2 : 1;
  struct rptqueue proctitle = { lbuf,
                                lbuf ? strlen(lbuf) : 0, 
                                lbuf, 
                                logpipe[0],  
                                1,
                                { -1, '\0' } };
  if (lbuf)
    rpt_init(&proctitle);

  for (;;) {
    x[0].fd = selfpipe[0];
    x[0].events = IOPAUSE_READ;
    x[1].fd = logpipe[0];
    x[1].events = IOPAUSE_READ;

    taia_uint(&deadline,3600);
    taia_now(&now);
    taia_add(&deadline,&now,&deadline);

    sig_unblock(sig_child);
    sig_unblock(sig_term);
    iopause(x,limit,&deadline,&now);
    sig_block(sig_child);
    sig_block(sig_term);

    /* output */
    if (x[1].revents & IOPAUSE_READ) 
      ret = rpt_status_update(&proctitle);

    while(read(selfpipe[0],&sig,1) > 0) 
      ;

    wpid = wait_nohang(&status);
    
    if (wpid == pid) {
      /* printf("status for %d - %d\n",wpid,status); */
      break;
    } 
    /* printf("status for %d\n",wpid); */
    if (term) {
      if (term == 1) {
        kill(pid,sig_term);
        kill(pid,sig_cont);
      }
      if (term == 2) kill(pid,sig_kill);
      if (term == 3) break;
      term++;
    }
  }
  fflush(stdout);
  _exit (0);
}


void usage(void) 
{
  const char usage[] = 
#include "buddy.usage.c"
    ;
  write(2,usage,sizeof(usage));
  exit(111);
}


int options_parse(int argc, char *argv[], char *envp[])
{
  int i,c;
  int option_index;
  int doing_log = 0;
  char *logbuf =  ".......................................";
  static struct option long_options[] = {
    { "djb",    1, 0, 'd' },
    { "skip-fd",     2, 0, 'f' },
    { 0, 0, 0, 0 }
  };

  if (argc < 2) { usage(); }
  while (1) {
    c = getopt_long (argc, argv, "d:l",
                     long_options, &option_index);

    switch (c) {
    case 'l':
      doing_log = 1;
    case 'd':
      djb_compat_dir = optarg;
      break;
    case 'f':
      parentfd = 0;
      break;
    default:
      break;
    }
    if (c == -1) { break; }
  }
  /* no logbuf */
  
  if (doing_log && (argc - optind == 1)) { 
    for (i = doing_log; i+1 < argc; i++) {
      argv[i] = argv[i+1];
    }
    argv[argc-1] = logbuf;
    execve(argv[0],argv,envp);
    strerr_die4sys(111,FATAL,"unable to restart self ",argv[0],": ");
  }
  if (optind == argc) { usage(); }

  if (argc > optind + 1) { lbuf = argv[argc-1]; }

  return optind;
}
