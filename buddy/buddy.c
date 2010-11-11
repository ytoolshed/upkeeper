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

#include "store/upk_db.h"
#include "common/rptqueue.h"
#include "common/sigblock.h"
#include "common/nonblock.h"

#define FATAL "buddy: fatal: "

static const char *event_path = NULL;
static int logp[2]            = {-1,-1};
static int sigp[2]            = {-1,-1};
static int eventfd            = -1;
static char * lbuf  = NULL;

int DEBUG = 0;


struct upk_srvc srvc = { NULL, NULL, NULL };

static char  *prog;
static char **envp;
static int  pid = 0;

int child = 0, term = 0;

static void sysdie(int e, const char *a,const char *b, const char *c)
{
  printf("%d",getpid());
  fprintf(stderr, "%s%s%s: %s\n",a,b,c,strerror(errno));
  exit(e);
}

static void handler(int sig)  
{ 
  switch (sig) {
  case SIGCHLD: child = 1 ; break;
  case SIGTERM: term  = 1 ; break;
  default: warn("unknown signal", sig);
  }
  sig = write(sigp[1]," ",1);
}

static void note_exit(int status) 
{
  if (lbuf) { close(logp[1]); }
  if (srvc.pdb) {
    upk_db_service_actual_status(&srvc, UPK_STATUS_VALUE_EXITED);
    if (term) {
      upk_db_service_actual_status(&srvc, UPK_STATUS_VALUE_STOP);
    }
  }
}


static void start_app(void) 
{

  if (lbuf) {
    if (pipe(logp) == -1) 
      sysdie(111,FATAL, "failed to create log pipe for ",prog);
    nonblock(logp[0]); nonblock(logp[1]);
    coe(logp[0]); coe(logp[1]);
  }

  if ((pid = fork()) == -1) 
    sysdie(111,FATAL, "failed to create pipe for ",prog);
  
  if (pid == 0) {
    upk_uncatch_signal(SIGCHLD);
    upk_uncatch_signal(SIGTERM);
    upk_unblock_signal(SIGCHLD);

    if (lbuf) {
      close(1);
      if (-1 == dup(logp[1])) {
        sysdie(111,FATAL"failed to dup stdout for ","",prog);
      }
      close(logp[1]);
    }
    execle(prog, prog, NULL, envp);

    sysdie(111,FATAL"failed to exec (",prog,")");
  }

  if (srvc.pdb) {
    upk_db_service_actual_status(&srvc, UPK_STATUS_VALUE_START);
    upk_db_service_pid(&srvc,pid);
  }

  if (lbuf) {
    close(logp[1]);
  }

}

  static void idle (void)
{

  char sig;
  int status, i, ret, wpid;
  int limit = lbuf ? 2 : 1;
  struct rptqueue proctitle = { lbuf,
                                lbuf ? strlen(lbuf) : 0,
                                lbuf,
                                logp[0],
                                1,
                                { -1, '\0' } };
  if (lbuf)
    rpt_init(&proctitle);

  upk_unblock_signal(SIGCHLD);
  for (;;) {
    struct timeval period;
    int retval;
    int maxfd = sigp[0];
    fd_set rfds;

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(sigp[0], &rfds);
    if (logp[0] != -1) {
      if (logp[0] > maxfd) { maxfd = logp[0]; }
      FD_SET(logp[0], &rfds);
    }
    period.tv_sec  = 60;
    period.tv_usec = 0;
    select(maxfd+1, &rfds, NULL, NULL, &period);

    /* output */
    if (logp[0] != -1 && FD_ISSET(logp[0], &rfds)) 
      ret = rpt_status_update(&proctitle);
      
    while(read(sigp[0],&sig,1) > 0) 
      ;

    upk_block_signal(SIGCHLD);
    upk_block_signal(SIGTERM);
    wpid = waitpid(-1,&status,WNOHANG);
    
    if (wpid == pid) {
      note_exit(status);
      if (term) break;
      start_app();
      sleep(1);
    } 

    upk_unblock_signal(SIGCHLD);
    upk_unblock_signal(SIGTERM);    

    if (term) {
      if (term == 1) {
        kill(pid,SIGTERM);
        kill(pid,SIGCONT);
      }
      if (term == 2) kill(pid,SIGKILL);
      if (term == 3) term--;
      term++;
    }
  }
  fflush(stdout);
  _exit (0);
}



int main(int ac, char **av, char **ep) 
{
  int i;
  envp = ep;
  prog = av[options_parse(ac,av,envp)];

  fprintf(stderr,"buddy 4 %s",prog);
  upk_catch_signal(SIGCHLD, handler);
  upk_catch_signal(SIGTERM, handler);
  upk_block_signal(SIGCHLD);

  if (pipe(sigp) == -1)
    sysdie(111,FATAL, "failed to create pipe for ",prog);

  coe(sigp[0]); nonblock(sigp[0]);
  coe(sigp[1]); nonblock(sigp[1]);
  
  start_app();
  if (write(3,";",1) != 1) {
    close(3);
  }
  idle();
}


void usage(void)
{
  const char usage[] = 
    "buddy: usage: buddy [--db /path/to/db -s service -p package] [-f] command\n";
  
  exit(111 + write(2,usage,sizeof(usage)));
}


int options_parse(int argc, char *argv[], char *envp[])
{
  int i,c;
  int option_index;
  int doing_log = 0;
  char *logbuf =  ".......................................";
  static struct option long_options[] = {
    { "fd",          2, 0, 'f' },
    { "package",     1, 0, 'p' },
    { "service",     1, 0, 's' },
    { "db",          1, 0, 'd' },
    { 0, 0, 0, 0 }
  };

  if (argc < 2) { usage(); }
  while (1) {
    c = getopt_long (argc, argv, "fd:s:p:l",
                     long_options, &option_index);

    switch (c) {
    case 'f':
      eventfd = 3;
      break;
    case 'l':
      doing_log = 1;
      break;
    case 'd':
      if (upk_db_init(optarg, &srvc.pdb)) {
        sysdie(111,FATAL"opening database (", optarg,") failed: ");
        printf("hey");
      }
      break;
    case 's':
      srvc.service = strdup(optarg);
      break;
    case 'p':
      srvc.package = strdup(optarg);
      break;
    default:
      break;
    }
    if (c == -1) { break; }
  }

  if (doing_log && (argc - optind == 1)) {
    for (i = doing_log; i+1 < argc; i++) {
      argv[i] = argv[i+1];
    }
    argv[argc-1] = logbuf;
    execve(argv[0],argv,envp);
    sysdie(111,FATAL,"unable to restart self ",argv[0]);
  }

  if (srvc.pdb && (!srvc.service || !srvc.package)) {  usage();  }

  if (optind == argc) { usage(); }

  if (argc > optind + 1) { lbuf = argv[argc-1]; }

  return optind;
}
