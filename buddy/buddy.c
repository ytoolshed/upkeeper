#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include "common/rptqueue.h"
#include "common/sigblock.h"
#include "common/nonblock.h"

#define FATAL "buddy: fatal: "

static const char *djb_compat_dir = NULL;
static       char *lbuf = NULL;

static int parentfd = 3;                         
static int logpipe[2] = { -1, -1 },
  selfpipe[2];

int child = 0, term = 0;
int pid;

static void sysdie(int e, const char *a,const char *b, const char *c)
{
  fprintf(stderr, "%s%s%s%s\n",a,b,c,strerror(errno));
  exit(e);
}


static void idle (void);
static void handler(int sig)  
{ 
  if (sig == SIGCHLD) child = 1;
  else term = 1;
  sig = write(selfpipe[1]," ",1); 
} 


int main(int ac, char **av, char **envp) 
{
  int i;
  char *prog = av[options_parse(ac,av,envp)];

  if (parentfd) {
    coe(parentfd);
    nonblock(parentfd);
  }

  upk_catch_signal(SIGCHLD, handler);
  upk_catch_signal(SIGTERM, handler);
  upk_block_signal(SIGCHLD);
  
  if (lbuf) {
    if (pipe(logpipe) == -1) 
      sysdie(111,FATAL, "failed to create log pipe for ",prog);

    nonblock(logpipe[0]); nonblock(logpipe[1]);
    coe(logpipe[0]); coe(logpipe[1]);
  }

  if (pipe(selfpipe) == -1)
    sysdie(111,FATAL, "failed to create pipe for ",prog);

  coe(selfpipe[0]); coe(selfpipe[1]);
  nonblock(selfpipe[0]);

  if ((pid = fork()) == -1) 
    sysdie(111,FATAL, "failed to create pipe for ",prog);
  

  if (pid == 0) {
    upk_uncatch_signal(SIGCHLD);
    upk_uncatch_signal(SIGTERM);
    upk_unblock_signal(SIGCHLD);

    if (lbuf) {
      if (dup2(logpipe[1],1) == -1)
        sysdie(111,FATAL, "failed to set log for ",prog);
      
      close(logpipe[1]);
      close(logpipe[0]);
    }
    execle(prog, prog, NULL, envp);
    
    sysdie(111,FATAL, "failed to exec ",prog);
  }

  if (lbuf) 
    close(logpipe[1]);
  
  (void)write(parentfd, &pid, 4);
    
  idle();
}

static void idle (void)
{

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
    struct timeval period;
    int retval;
    fd_set rfds;
    
    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(selfpipe[0], &rfds);
    FD_SET(logpipe[0], &rfds);
    period.tv_sec=60;

    /* output */
    if (FD_ISSET(logpipe[0], &rfds)) 
      ret = rpt_status_update(&proctitle);
      
    while(read(selfpipe[0],&sig,1) > 0) 
      ;

    wpid = waitpid(-1,&status,WNOHANG);
    
    if (wpid == pid) {
      printf("status for %d - %d\n",wpid,status);
      break;
    } 

    if (term) {
      if (term == 1) {
        kill(pid,SIGTERM);
        kill(pid,SIGCONT);
      }
      if (term == 2) kill(pid,SIGKILL);
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
    "buddy: usage: buddy [-d dir] [-f] command";
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
    { "fd",     2, 0, 'f' },
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
    sysdie(111,FATAL,"unable to restart self ",argv[0]);
  }
  if (optind == argc) { usage(); }

  if (argc > optind + 1) { lbuf = argv[argc-1]; }

  return optind;
}
