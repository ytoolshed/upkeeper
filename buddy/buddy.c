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
static       char *lbuf = NULL;


struct eventfd {
  int fd;
  enum { SOCKET, NOT_SOCKET, DUNNO } fdt;
};

struct eventfd eventfd = { -1, DUNNO };


int DEBUG = 0;


struct upk_srvc srvc = { NULL, NULL, NULL };

static const char *djb_compat_dir = NULL;
static       char *lbuf           = NULL;

static char  *prog;
static char **envp;
static int  pid = 0;

static int parentfd = -1;                         
static int logp[2]  = 0;
static int sigp[2] = 0;

int child = 0, term = 0;

static ssize_t event_write (struct eventfd *fd, const void * data, size_t len )
{
  ssize_t ret;

  switch (fd->fdt) {
  case SOCKET:
    return send(fd->fd, data, len, 0);
  case NOT_SOCKET:
    return write(fd->fd, data, len);
  case DUNNO:
  default:
    ret = send(fd->fd, data, len, 0);
    if (ret == -1 && errno == ENOTSOCK) {
      fd->fdt = NOT_SOCKET;
      return event_write(fd,data,len);
    }
    fd->fdt = SOCKET;
    return ret;
  }
}

static void sysdie(int e, const char *a,const char *b, const char *c)
{
  fprintf(stderr, "%s%s%s%s\n",a,b,c,strerror(errno));
  exit(e);
}


static void idle (void);
static void handler(int sig)  
{ 
  switch (sig) {
  case SIGCHILD: child = 1 ; break;
  case  SIGTERM: term  = 1 ; break;
  default: warn("unknown signal", sig);
  }
  sig = write(sigp[1]," ",1);
}

static void note_exit(int status) 
{
  if (srvc.pdb) {
    upk_db_service_actual_status(&srvc, UPK_STATUS_VALUE_STOP);
  }
}


static void start_child(void) 
{
  if ((pid = fork()) == -1) 
    sysdie(111,FATAL, "failed to create pipe for ",prog);
  
  if (pid == 0) {
    upk_uncatch_signal(SIGCHLD);
    upk_uncatch_signal(SIGTERM);
    upk_unblock_signal(SIGCHLD);

    if (lbuf) {
      if (dup2(logp[1],1) == -1)
        sysdie(111,FATAL, "failed to set log for ",prog);
      
      close(logp[1]);
      close(logp[0]);
    }
    execle(prog, prog, NULL, envp);

    sysdie(111,FATAL"failed to exec ",prog, ": ");
  }

  if (srvc.pdb) {
    upk_db_service_actual_status(&srvc, UPK_STATUS_VALUE_START);
  }

  if (lbuf)
    close(logp[1]);
  
  if (eventfd.fd != -1) {   event_write(&eventfd, &pid, 4);  }
}

int main(int ac, char **av, char **ep) 
{
  int i;
  envp = ep;
  prog = av[options_parse(ac,av,envp)];

  if (parentfd) {
    coe(parentfd);
    nonblock(parentfd);
  }

  upk_catch_signal(SIGCHLD, handler);
  upk_catch_signal(SIGTERM, handler);
  upk_block_signal(SIGCHLD);
  
  if (lbuf) {
    if (pipe(logp) == -1) 
       sysdie(111,FATAL, "failed to create log pipe for ",prog);

    nonblock(logp[0]); nonblock(logp[1]);
    coe(logp[0]); coe(logp[1]);
  }

  if (pipe(sigp) == -1)
    sysdie(111,FATAL, "failed to create pipe for ",prog);

  coe(sigp[0]); coe(sigp[1]);
  nonblock(sigp[0]);

  start_child();
  idle();
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

    wpid = waitpid(-1,&status,WNOHANG);
    
    if (wpid == pid) {
      note_exit(status);
      start_child();
      sleep(1);
    } 

    upk_unblock_signal(SIGCHLD);    

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
    "buddy: usage: buddy [--db /path/to/db -s service -p package] [-f] command\n";
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
    { "fd",          2, 0, 'f' },
    { "package",     1, 0, 'p' },
    { "service",     1, 0, 's' },
    { "db",          1, 0, 'd' },
    { 0, 0, 0, 0 }
  };

  if (argc < 2) { usage(); }
  while (1) {
    c = getopt_long (argc, argv, "d:s:p:l",
                     long_options, &option_index);

    switch (c) {
    case 'l':
      doing_log = 1;
    case 'd':
      if (upk_db_init(optarg, &srvc.pdb)) {
        sysdie(111,FATAL"opening database (", optarg,") failed: ");
      }
      break;
    case 's':
      srvc.service = strdup(optarg);
      break;
    case 'p':
      srvc.package = strdup(optarg);
      break;
    case 'f':
      eventfd.fd = 3;
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
