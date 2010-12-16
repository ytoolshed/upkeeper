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
#include "common/rptqueue.h"
#include "common/sigblock.h"
#include "common/nonblock.h"
#include "common/err.h"

#define FATAL "buddy: fatal: "
#define ERROR "buddy: error: "
static int logp[2]            = {-1,-1};
static int sigp[2]            = {-1,-1};
static int eventsock          = -1;
static int eventfd[2]         = {-1,-1};
static char * lbuf  = NULL;
static int quiet = 0;
static int done  = 0;
static int last_status = -1;
static struct rptqueue proctitle;

int DEBUG = 0;

int buddy_pid;

char buddy_sock[108];

static char  **prog;
static char **envp;
static int  pid = 0;

int child = 0, term = 0;

static void handler(int sig)  
{ 
  switch (sig) {
  case SIGCHLD: child = 1 ; break;
  case SIGTERM: term = 1 ; done = 1; break;
  default: warn("unknown signal", sig);
  }
  sig = write(sigp[1]," ",1);
}

static void send_message(int fd, const char *msg, int size) 
{
  
  int *efd = eventfd; int *end = eventfd + sizeof(eventfd) / sizeof(eventfd[0]);
  if (fd != -1) { efd = &fd; end = &fd; }
  do {
    if (*efd != -1 && send(*efd, msg, size, 0) == -1) {
      syswarn3(ERROR,"write to controller failed","");
    }
  } while(++efd < end);
}

static void send_status(int fd, char m) 
{
  char msg[sizeof(int)*3+1];
  msg[0] = m;
  memcpy(msg+1,&buddy_pid,sizeof(int));
  memcpy(msg+sizeof(int)+1,&pid,sizeof(int));
  memcpy(msg+sizeof(int)*2+1,&last_status,sizeof(int));
  send_message(fd, msg,sizeof(int)*3 + 1);
}


static void start_app(void) 
{
  char msg[sizeof(int)*2+1] = "u";
  if (lbuf) {
    if (pipe(logp) == -1) {
      syswarn3(ERROR, "failed to create log pipe for ",*prog);
      return;
    }
    nonblock(logp[0]); nonblock(logp[1]);
    coe(logp[0]); coe(logp[1]);
  }

  if ((pid = fork()) == -1) {
    syswarn3(ERROR, "failed to create pipe for ",*prog);
    return;
  }

  if (pid == 0) {
    upk_uncatch_signal(SIGCHLD);
    upk_uncatch_signal(SIGTERM);
    upk_unblock_signal(SIGCHLD);

    if (lbuf) {
      close(1);
      if (-1 == dup(logp[1])) {
        sysdie3(111,FATAL"failed to dup stdout for ","",*prog);
      }
      close(logp[1]);
    }

    execvp(*(prog+2), prog+2);
    if (errno == ENOENT) {
      execvp(*prog, prog);
    }
    sysdie3(111,FATAL"exec (",*prog,") error");
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
  send_status(-1,'u');
}

static void idle (void)
{

  char sig, ev[2] = { 0 };
  int status, i, ret, wpid;
  int limit = lbuf ? 2 : 1;

  upk_unblock_signal(SIGCHLD);
 
  for (;;) {
    struct timeval period;
    int retval;
    int maxfd = sigp[0];
    fd_set rfds;


    FD_ZERO(&rfds);
    FD_SET(sigp[0], &rfds);
    FD_SET(eventsock, &rfds);
    if (eventsock > maxfd)   { maxfd = eventsock; }
    for (i = 0; i < sizeof(eventfd)/sizeof(int);  i++) {
      if (eventfd[i] > 0)  { 
        FD_SET(eventfd[i], &rfds);
        if (eventfd[i] > maxfd) { maxfd = eventfd[i]; }
      }
    }

    /* for output from monitored process */
    if (logp[0] != -1) {
      FD_SET(logp[0], &rfds);
      if (logp[0] > maxfd) { maxfd = logp[0]; }
    }

    period.tv_sec  = 60;
    period.tv_usec = 0;
    select(maxfd+1, &rfds, NULL, NULL, &period);

    while(read(sigp[0],&sig,1) > 0) 
      ;

    /* output */
    if (logp[0] != -1 && FD_ISSET(logp[0], &rfds)) 
      ret = rpt_status_update(&proctitle);

    if (FD_ISSET(eventsock, &rfds)) {
      int *fd = eventfd;
      do {
        if (*fd == -1) { 
          *fd = accept(eventsock,NULL,0); 
          nonblock(*fd); 
          break; 
        }
        fd++;
      } while(fd < eventfd + sizeof(eventfd)/sizeof(eventfd[0]));
    }

    for (i = 0; i < sizeof(eventfd)/sizeof(eventfd[0]); i++) {
      int res;
      if (eventfd[i] == -1) { continue; }

      switch(read(eventfd[i],&ev,1)) {
      case -1:
        if (errno == EINTR || errno == EAGAIN) 
          break;
      case  0:
        close(eventfd[i]);
        eventfd[i] = -1;
        break;
      default:
        switch(ev[0]) {
        case 's':
          send_status(eventfd[i], pid == -1 ? 'd' : 'u');
          break;
        case 't':
          term = 1;
          done = 1;
          unlink(buddy_sock);
          break;
        default:
          syswarn3(ERROR, "unknown command:",ev);
        }
        break;
      }
    }

    upk_block_signal(SIGCHLD);
    upk_block_signal(SIGTERM);
    wpid = waitpid(-1,&status,WNOHANG);
    
    if (wpid == pid) {
      last_status = status;
      pid = -1;
      send_status(-1,'d');
      if (lbuf) { close(logp[0]); }
      if (done) break;
      start_app();
      sleep(1);
    } 

    upk_unblock_signal(SIGCHLD);
    upk_unblock_signal(SIGTERM);    
    if (term) {
      unlink(buddy_sock);
      if (term == 1) {
        kill(pid,SIGTERM);
        kill(pid,SIGCONT);
      }
      if (term == 2) kill(pid,SIGKILL);
      if (term == 3) term--;
      term++;
    }
  }
  send_status(-1,'e');
  exit (0);
}



int main(int ac, char **av, char **ep) 
{
  int i;
  int optind = options_parse(ac,av,envp);
  prog = malloc(ac-optind + 3 * sizeof(char *));
  static struct sockaddr_un eventaddr;

  if (!prog)
    sysdie3(111,FATAL, "failed to malloc command line ",*prog);

  prog[0] = "/bin/sh";
  prog[1] = "-c";
  for (i = 2; optind < ac; optind++) {
    prog[i++] = av[optind];
  }

  upk_catch_signal(SIGCHLD, handler);
  upk_catch_signal(SIGTERM, handler);
  upk_block_signal(SIGCHLD);
  buddy_pid = getpid();

  if (pipe(sigp) == -1)
    sysdie3(111,FATAL, "failed to create pipe for ",*prog);

  coe(sigp[0]); nonblock(sigp[0]);
  coe(sigp[1]); nonblock(sigp[1]);

  eventsock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (eventsock == -1) 
    sysdie3(111,FATAL,"socket failed","");
  eventaddr.sun_family = AF_UNIX;

  sprintf(buddy_sock,"./.buddy.%d",buddy_pid);
  strcpy(eventaddr.sun_path,buddy_sock);
  unlink(eventaddr.sun_path);
  
  if (bind(eventsock,(struct sockaddr *)&eventaddr, SUN_LEN(&eventaddr))==-1)
    sysdie3(111,FATAL,"binding to socket - ",eventaddr.sun_path);
  
  if (listen(eventsock,2) == -1)
    sysdie3(111,FATAL,"listening on socket - ",eventaddr.sun_path);

  nonblock(eventsock);

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
    "buddy: usage: buddy [-o] command\n";
  
  exit(111 + write(2,usage,sizeof(usage)));
}


int options_parse(int argc, char *argv[], char *envp[])
{
  int i,c;
  int option_index;
  int doing_log = 0;
  char *logbuf =  ".......................................";
  static struct option long_options[] = {
    { "once",        1, 0, 'o' },
    { "quiet",	     1, 0, 'q' },
    { "log",	     1, 0, 'l' },
    { 0, 0, 0, 0 }
  };

  if (argc < 2) { usage(); }
  while (1) {
    c = getopt_long (argc, argv, "oql",
                     long_options, &option_index);

    switch (c) {
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
    default:
      break;
    }
    if (c == -1) { break; }
  }

  if (doing_log) {
    argv[doing_log-1] = logbuf;
    execve(argv[0],argv,envp);
    sysdie3(111,FATAL,"unable to restart self ",argv[0]);
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
