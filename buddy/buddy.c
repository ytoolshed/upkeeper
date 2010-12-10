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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "store/upk_db.h"
#include "common/rptqueue.h"
#include "common/sigblock.h"
#include "common/nonblock.h"

#define FATAL "buddy: fatal: "
#define ERROR "buddy: error: "

static int logp[2]            = {-1,-1};
static int sigp[2]            = {-1,-1};
static int eventfd            = -1;
static char * lbuf  = NULL;
static int quiet = 1;
static int done  = 0;
static int parentfd           = -1;
static struct rptqueue proctitle;
static struct sockaddr_un controller;

int DEBUG = 0;

int buddy_pid;

static char  **prog;
static char **envp;
static int  pid = 0;

int child = 0, term = 0;

static void sysdie(int e, const char *a,const char *b, const char *c)
{
  if (quiet < 2 || pid == 0) {
    fprintf(pid == 0 ? stdout : stderr, "%s%s%s: %s\n",a,b,c,strerror(errno));
  }
  if (quiet) {
    /* XXX: should write errors into db or to domain socket? */
  }
  exit(e);
}

static void syswarn(const char *a,const char *b, const char *c)
{
  if (quiet < 2 || pid == 0) {
    fprintf(pid == 0 ? stdout : stderr, "%s%s%s: %s\n",a,b,c,strerror(errno));
  }
}

static void handler(int sig)  
{ 
  switch (sig) {
  case SIGCHLD: child = 1 ; break;
  case SIGTERM: term  = 1 ; done = 1 ; break;
  default: warn("unknown signal", sig);
  }
  sig = write(sigp[1]," ",1);
}

static void send_message(const char *msg, int size) 
{
  if (sendto(eventfd,msg,size,0,(struct sockaddr *)&controller,
             SUN_LEN(&controller)) == -1
      && !quiet) {
    syswarn(ERROR,"write to controller failed","");
  }
}

static void note_exit(int status) 
{
  char msg[sizeof(int)*2+1] = "d";
  
  memcpy(msg+1,&buddy_pid,sizeof(int));
  memcpy(msg+sizeof(int)+1,&status,sizeof(int));
  send_message(msg,sizeof(int)*2 + 1);
  if (lbuf) { close(logp[0]); }

}


static void start_app(void) 
{
  char msg[sizeof(int)*2+1] = "u";
  if (lbuf) {
    if (pipe(logp) == -1) {
      syswarn(ERROR, "failed to create log pipe for ",*prog);
      return;
    }
    nonblock(logp[0]); nonblock(logp[1]);
    coe(logp[0]); coe(logp[1]);
  }

  if ((pid = fork()) == -1) {
    syswarn(ERROR, "failed to create pipe for ",*prog);
    return;
  }

  if (pid == 0) {
    upk_uncatch_signal(SIGCHLD);
    upk_uncatch_signal(SIGTERM);
    upk_unblock_signal(SIGCHLD);

    if (lbuf) {
      close(1);
      if (-1 == dup(logp[1])) {
        sysdie(111,FATAL"failed to dup stdout for ","",*prog);
      }
      close(logp[1]);
    }

    execvp(*(prog+2), prog+2);
    if (errno == ENOENT) {
      execvp(*prog, prog);
    }
    sysdie(111,FATAL"exec (",*prog,") error");
  }

  if (lbuf) {
    close(logp[1]);

    proctitle.display = lbuf;
    proctitle.dlen    = lbuf ? strlen(lbuf) : 0;
    proctitle.dpos    = lbuf;
    proctitle.ifd     = logp[0];
    proctitle.ofd     = quiet ? -1 : 1;
    proctitle.buf[1]  = 0;
    rpt_init(&proctitle);
  }
  memcpy(msg+1,&buddy_pid,sizeof(int));
  memcpy(msg+sizeof(int)+1,&pid,sizeof(int));
  send_message(msg,sizeof(int)*2+1);
}

  static void idle (void)
{

  char sig;
  int status, i, ret, wpid;
  int limit = lbuf ? 2 : 1;

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
      if (done) break;
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
  exit (0);
}



int main(int ac, char **av, char **ep) 
{
  int i;
  int optind = options_parse(ac,av,envp);
  prog = malloc(ac-optind + 3 * sizeof(char *));

  if (!prog)
    sysdie(111,FATAL, "failed to malloc command line ",*prog);

  prog[0] = "/bin/sh";
  prog[1] = "-c";
  for (i = 2; optind < ac; optind++) {
    prog[i++] = av[optind];
  }

  upk_catch_signal(SIGCHLD, handler);
  upk_catch_signal(SIGTERM, handler);
  upk_block_signal(SIGCHLD);

  if (pipe(sigp) == -1)
    sysdie(111,FATAL, "failed to create pipe for ",*prog);

  coe(sigp[0]); nonblock(sigp[0]);
  coe(sigp[1]); nonblock(sigp[1]);

  buddy_pid = getpid();
  start_app();
  idle();
}

static int is_logbuf(const char *b) {
  do {
    if (*b != '.') return 0;
  } while (*++b);
  return 1;
}

void usage(void)
{
  const char usage[] = 
    "buddy: usage: buddy [--sock /path/to/socket] [-f] command\n";
  
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
    { "socket",      1, 0, 's' },
    { "once",        1, 0, 'o' },
    { "quiet",	     1, 0, 'q' },
    { 0, 0, 0, 0 }
  };

  if (argc < 2) { usage(); }
  while (1) {
    c = getopt_long (argc, argv, "ovfql::d:s:p:",
                     long_options, &option_index);

    switch (c) {
    case 'f':
      parentfd = 3;
      break;
    case 'l':
      doing_log = optind;
      break;
    case 'o':
      done = 1;
      break;
    case 'q':
      quiet++;
      break;
    case 'v':
      quiet--;
      DEBUG = 1;
      break;
    case 's':
      eventfd = socket(AF_UNIX, SOCK_DGRAM, 0);
      nonblock(eventfd); coe(eventfd);
      if (eventfd == 1) 
        sysdie(111,FATAL,"socket failed","");
      controller.sun_family = AF_UNIX;
      strcpy(controller.sun_path,optarg);
      break;
    default:
      break;
    }
    if (c == -1) { break; }
  }

  if (doing_log) {
    argv[doing_log] = logbuf;
    execve(argv[0],argv,envp);
    sysdie(111,FATAL,"unable to restart self ",argv[0]);
  }

  if (optind == argc) { usage(); }
  if (argv[optind][0] == '-' && argv[optind][1] == '-'  && argv[optind][2] == 0) {
    optind++;
  }
  if (is_logbuf(argv[optind])) {
    lbuf = argv[optind++];
  }
  return optind;
}
