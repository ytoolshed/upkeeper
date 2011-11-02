
/****************************************************************************
 * Copyright (c) 2011 Yahoo! Inc. All rights reserved. Licensed under the
 * Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License
 * at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable
 * law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 * See accompanying LICENSE file. 
 ************************************************************************** */

#include "buddy.h"
#include <assert.h>
#include <stdarg.h>
#include <upkeeper/upk_error.h>

#define BUDDY_RETRY_TIMEOUT_SEC 1
#define BUDDY_RETRY_TIMEOUT_NSEC 0

#define BUDDY_SELECT_TIMEOUT_SEC 1
#define BUDDY_SELECT_TIMEOUT_USEC 0                        // 0000

#define RESPAWN_WINDOW 5                                   /* seconds */
#define MAX_RESPAWN_COUNT 10                               /* max number of times a process may restart
                                                              within the restart window before mandatory
                                                              ratelimit is applied */
#define RESPAWN_RATELIMIT 300                              /* seconds to wait before trying to respawn again
                                                              after respawning too fast */

/* exportable */
// char buddy_uuid[37] = "";
upk_uuid_t              buddy_uuid;
char                   *buddy_service_name = NULL;
char                    buddy_root_path[BUDDY_MAX_PATH_LEN] = DEFAULT_BUDDY_ROOT;
uid_t                   buddy_setuid;
gid_t                   buddy_setgid;
size_t                  ringbuffer_size = 32;
char                  **proc_envp = NULL;
long                    reconnect_retries = 5;
bool                    randomize_ratelimit = false;
uint32_t                user_ratelimit = 0;

void                    buddy_init(diag_output_callback_t logger);
int32_t                 buddy_event_loop(void);
void                    buddy_cleanup(void);

/* not exportable */
static time_t           respawn_timestamp = 0;
static uint32_t         rapid_respawn_count = 0;
static bool             force_ratelimit = false;

static size_t           ringbuffer_pending = 0;
static bool             chld_terminated = false;
static pid_t            proc_pid = 0;
static buddy_cmnd_t     last_command = UPK_CTRL_NONE;
static buddy_cmnd_t     this_command = UPK_CTRL_NONE;
static int              buddy_shutdown = 0;
static buddy_info_t    *info_ringbuffer = NULL;

static int              buddy_sockfd = -1;
static int              buddy_ctrlfd = -1;

static char             buddy_path_buf[BUDDY_MAX_PATH_LEN] = "";
static char             buddy_path_prefix[BUDDY_MAX_PATH_LEN] = "";
static char             buddy_string_buf[BUDDY_MAX_PATH_LEN] = "";

static struct sockaddr_un buddy_sockaddr = { 0 };
static socklen_t        buddy_sockaddr_len = 0;

static int              sockopts = 0;
static int              highest_fd = 0;
static struct timeval   timeout = {.tv_sec = BUDDY_SELECT_TIMEOUT_SEC,.tv_usec = BUDDY_SELECT_TIMEOUT_USEC };
static struct timespec  nanotimeout = {.tv_sec = BUDDY_RETRY_TIMEOUT_SEC,.tv_nsec =
        BUDDY_RETRY_TIMEOUT_NSEC };
static size_t           ncount = 0;

static buddy_runstate_t desired_state = BUDDY_STOPPED;


/* functions */

static inline void      buddy_init_paths(void);
static inline char     *buddy_path(const char *suffixfmt, ...);
static inline void      buddy_init_socket(void);
static inline bool      buddy_phone_home(void);
static inline void      buddy_record_state(void);
static inline void      buddy_zero_info(void);
static inline void      buddy_init_ringbuffer(void);
static inline void      buddy_free_ringbuffer(void);
static inline void      buddy_accept(void);
static inline int       buddy_build_fd_set(fd_set * socks, bool listen_sock);
static inline bool      buddy_handle_flags(void);
static inline void      buddy_stop_proc(void);
static inline void      buddy_reload_proc(void);
static inline void      buddy_cust_action(uint32_t act_num);
static inline void      buddy_handle_command(void);
static inline void      buddy_setreguid(void);
static inline void      buddy_setup_fds(void);
static inline int32_t   read_ctrl(buddy_cmnd_t * buf);
static inline int32_t   write_ctrl(buddy_info_t * buf);

static pid_t            buddy_exec_action(void);
static bool             buddy_start_proc(void);
static void             resolve_symlink(void);
static void             buddy_flush_ringbuffer(void);
static void             sa_sigaction_func(int signal, siginfo_t * siginfo, void *ucontext);

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static void
sa_sigaction_func(int signal, siginfo_t * siginfo, void *ucontext)
{
    static pid_t            pid;
    static int              proc_waitstatus = 0;

    pid = 0;

    switch (signal) {
    case SIGCHLD:
        while((pid = waitpid(-1, &proc_waitstatus, WNOHANG)) > 0) {
            if(pid == proc_pid) {
                upk_debug1("SIGCHLD for monitored process received, recording\n");
                chld_terminated = true;
                proc_pid = 0;
            }
            buddy_record_state();
            info_ringbuffer->siginfo = *siginfo;
            info_ringbuffer->wait_pid = pid;
            info_ringbuffer->wait_status = proc_waitstatus;
            info_ringbuffer = info_ringbuffer->next;
        }
        break;
    default:
        buddy_shutdown = signal;
        break;
    }
    return;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static char            *
buddy_path(const char *suffixfmt, ...)
{
    static va_list          ap;

    memset(buddy_string_buf, 0, sizeof(buddy_string_buf));

    va_start(ap, suffixfmt);
    vsnprintf(buddy_string_buf, sizeof(buddy_string_buf) - 1, suffixfmt, ap);
    va_end(ap);

    memset(buddy_path_buf, 0, sizeof(buddy_path_buf));
    snprintf(buddy_path_buf, sizeof(buddy_path_buf) - 1, "%s/%s", buddy_path_prefix, buddy_string_buf);

    return buddy_path_buf;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
buddy_init_paths(void)
{
    if(strnlen(buddy_root_path, BUDDY_MAX_PATH_LEN) == BUDDY_MAX_PATH_LEN)
        upk_fatal("buddy root path is too long, cannot continue");

    if(strnlen(buddy_service_name, UPK_MAX_STRING_LEN) > UPK_MAX_STRING_LEN)
        upk_fatal("service name is too long, cannot continue");

    if((strnlen(buddy_root_path, BUDDY_MAX_PATH_LEN) + strnlen(buddy_service_name, UPK_MAX_STRING_LEN)) >
       BUDDY_MAX_PATH_LEN)
        upk_fatal("compined path length of buddy root and service name is too long, cannot continue");

    /* "actions/reload" should be the longest string we ever try to assemble */
    if((strnlen(buddy_root_path, BUDDY_MAX_PATH_LEN) + strnlen(buddy_service_name, UPK_MAX_STRING_LEN) +
        strlen("actions/reload")) > BUDDY_MAX_PATH_LEN)
        upk_fatal("compined path length of buddy root and service name is too long, cannot continue");

    snprintf(buddy_path_prefix, sizeof(buddy_path_prefix) - 1, "%s/%s", buddy_root_path, buddy_service_name);
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
buddy_init_socket(void)
{
    buddy_sockaddr_len = sizeof(buddy_sockaddr);

    (void) unlink(buddy_path("buddy.sock"));
    mode_t                  oldumask;

    upk_debug0("initalizing socket %s\n", buddy_path("buddy.sock"));

    strncpy(buddy_sockaddr.sun_path, (const char *) buddy_path("buddy.sock"),
            sizeof(buddy_sockaddr.sun_path) - 1);
    buddy_sockaddr.sun_family = AF_UNIX;

    oldumask = umask(077);
    buddy_sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    assert(buddy_sockfd >= 0);
    assert(fcntl(buddy_sockfd, F_GETFL) >= 0);

    assert(bind(buddy_sockfd, (struct sockaddr *) &buddy_sockaddr, sizeof(buddy_sockaddr)) == 0);
    assert(listen(buddy_sockfd, SOMAXCONN) == 0);
    fcntl(buddy_sockfd, F_SETFD, FD_CLOEXEC);
    umask(oldumask);
}



/* ********************************************************************************************************************
   ******************************************************************************************************************* */
void
buddy_init(diag_output_callback_t logger)
{
    sigset_t                sigset;
    struct sigaction        sigact;

    (void) upk_reg_diag_callback(logger);
    upk_verbose("Starting buddy initialization...\n");
    buddy_init_paths();

    chdir("/");
    fclose(stdin);

    buddy_ctrlfd = -1;
    buddy_sockfd = -1;

#ifdef HAVE_ATEXIT
    atexit(buddy_cleanup);
#endif

    /* TODO: use capabilities CAP_SETUID/CAP_SETGID tests on platforms that provide it */
    if(buddy_setgid != getgid() || buddy_setuid != getuid())
        if(geteuid() != 0)
            upk_warn("Cannot setuid/setgid, because buddy is not running as root\n");

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGCHLD);

    sigfillset(&sigact.sa_mask);
    sigact.sa_flags = SA_NOCLDSTOP | SA_RESTART | SA_SIGINFO;
    sigact.sa_sigaction = sa_sigaction_func;

    sigaction(SIGCHLD, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGINT, &sigact, NULL);

    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = SIG_IGN;

    sigaction(SIGPIPE, &sigact, NULL);

    buddy_init_ringbuffer();
    buddy_init_socket();

    upk_notice("Buddy initialized\n");
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline           bool
buddy_handle_flags(void)
{
    if(buddy_shutdown > 0)
        return true;

    if(chld_terminated && desired_state == BUDDY_RUNNING) {
        if(force_ratelimit && ((respawn_timestamp + RESPAWN_RATELIMIT) > time(NULL)))
            return false;
        else
            force_ratelimit = false;

        upk_notice("Child terminated, restarting..\n");
        if(buddy_start_proc()) {
            chld_terminated = false;
        }
    }
    return false;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
void
commit_buddycide(int32_t signum)
{
    static sigset_t         sigset, oldset;
    struct sigaction        sigact;

    sigfillset(&sigset);
    sigprocmask(SIG_BLOCK, &sigset, &oldset);

    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = SIG_DFL;

    sigaction(SIGCHLD, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGPIPE, &sigact, NULL);

    buddy_cleanup();

    sigemptyset(&sigset);
    sigprocmask(SIG_SETMASK, &sigset, NULL);
    upk_debug1("Exiting on signal %d\n", signum);
    kill(getpid(), signum);
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
int32_t
buddy_event_loop(void)
{
    static fd_set           readset;

    while(1) {
        timeout.tv_sec = BUDDY_SELECT_TIMEOUT_SEC;
        timeout.tv_usec = BUDDY_SELECT_TIMEOUT_USEC;

        this_command = 0;

        highest_fd = buddy_build_fd_set(&readset, true);

        if(select(highest_fd + 1, &readset, NULL, NULL, &timeout) > 0) {
            if(FD_ISSET(buddy_sockfd, &readset)) {
                buddy_accept();
                continue;
            }

            if((buddy_ctrlfd > 0) && FD_ISSET(buddy_ctrlfd, &readset)) {
                read_ctrl(&this_command);
                if(this_command) {
                    buddy_handle_command();
                    upk_debug1("replying to controler by flushing ringbuffer\n");
                    buddy_flush_ringbuffer();
                }
            }
        }

        if(buddy_handle_flags())
            break;
    }
    if(buddy_shutdown && buddy_shutdown != SIGHUP)
        commit_buddycide(buddy_shutdown);

    return 0;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
void
buddy_cleanup(void)
{
    unlink(buddy_path("buddy.sock"));

    if(proc_pid != 0)
        buddy_stop_proc();

    if(info_ringbuffer) {
        buddy_flush_ringbuffer();
        buddy_free_ringbuffer();
    }

    if(buddy_service_name) {
        free(buddy_service_name);
        buddy_service_name = NULL;
    }
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline           bool
buddy_phone_home(void)
{
    buddy_path("controller.sock");
    resolve_symlink();

    strncpy(buddy_sockaddr.sun_path, buddy_string_buf, sizeof(buddy_sockaddr.sun_path) - 1);
    buddy_sockaddr.sun_family = AF_UNIX;

    buddy_ctrlfd = socket(PF_UNIX, SOCK_STREAM, 0);

    if(connect(buddy_ctrlfd, (struct sockaddr *) &buddy_sockaddr, buddy_sockaddr_len) == 0)
        return true;

    return false;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
buddy_record_state(void)
{
    upk_debug1("Recording state\n");

    buddy_zero_info();
    ringbuffer_pending = (ringbuffer_pending >= ringbuffer_size) ? ringbuffer_size : ringbuffer_pending + 1;
    info_ringbuffer->uuid = buddy_uuid;
    info_ringbuffer->service_pid = proc_pid;
    info_ringbuffer->command = last_command;
    info_ringbuffer->desired_state = desired_state;
    info_ringbuffer->timestamp = time(NULL);
    info_ringbuffer->populated = true;
}


/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
buddy_zero_info(void)
{
    upk_debug1("Clearing ringbuffer slot\n");
    if(info_ringbuffer->populated)
        ringbuffer_pending = (ringbuffer_pending <= 1) ? 0 : ringbuffer_pending - 1;
    memset(info_ringbuffer, 0,
           sizeof(*info_ringbuffer) - sizeof(info_ringbuffer->next) - sizeof(info_ringbuffer->slot_n));
}


/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
buddy_init_ringbuffer(void)
{
    uint32_t                n = 0;
    buddy_info_t           *first_node = NULL;

    assert((first_node = calloc(1, sizeof(*first_node))));
    info_ringbuffer = first_node;
    info_ringbuffer->slot_n = 0;

    for(n = 1; n < ringbuffer_size; n++) {
        assert((info_ringbuffer->next = calloc(1, sizeof(*info_ringbuffer->next))));
        info_ringbuffer->slot_n = n;
        info_ringbuffer = info_ringbuffer->next;
    }
    info_ringbuffer->next = first_node;

    return;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
buddy_free_ringbuffer(void)
{
    uint32_t                n = 0;
    buddy_info_t           *nodep = NULL;

    for(n = 0; n < ringbuffer_size; n++) {
        nodep = info_ringbuffer->next;
        free(info_ringbuffer);
        info_ringbuffer = nodep;
    }
    info_ringbuffer = NULL;
    return;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
buddy_accept(void)
{
    upk_debug1("accepting new connection from a controller\n");

    /* XXX: Is this the correct behavior? drop anybody we have connected in lieu of a new caller? -- JB */
    if(buddy_ctrlfd > 0) {
        upk_debug1("Dropping existing connection\n");
        close(buddy_ctrlfd);
    }

    buddy_ctrlfd = accept(buddy_sockfd, (struct sockaddr *) &buddy_sockaddr, &buddy_sockaddr_len);

    if(buddy_ctrlfd > 0 && (sockopts = fcntl(buddy_ctrlfd, F_GETFL))) {
        fcntl(buddy_ctrlfd, F_SETFL, sockopts | O_NONBLOCK);
        fcntl(buddy_ctrlfd, F_SETFD, FD_CLOEXEC);
        upk_debug1("accepted new controller connection\n");
    } else {
        upk_debug1("accept failed\n");
        close(buddy_ctrlfd);
        buddy_ctrlfd = -1;
    }
}


/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline           bool
buddy_disconnect(size_t count)
{
    if(count < 1 && !(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
        upk_debug1("Disconnecting controller: %s\n", strerror(errno));
        shutdown(buddy_ctrlfd, SHUT_RDWR);
        close(buddy_ctrlfd);
        buddy_ctrlfd = -1;
        return true;
    }
    return false;
}


/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline int
buddy_build_fd_set(fd_set * socks, bool listen_sock)
{
    FD_ZERO(socks);

    if(listen_sock && buddy_sockfd >= 0)
        FD_SET(buddy_sockfd, socks);
    if(buddy_ctrlfd > 0)
        FD_SET(buddy_ctrlfd, socks);

    return (buddy_sockfd > buddy_ctrlfd) ? buddy_sockfd : buddy_ctrlfd;
}


/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static                  pid_t
buddy_exec_action(void)
{
    static pid_t            pid = 0;
    static sigset_t         sigset, oldset;
    struct sigaction        sigact;
    static struct stat      buf;
    static char             path_buf[BUDDY_MAX_PATH_LEN];

    sigfillset(&sigset);
    sigprocmask(SIG_BLOCK, &sigset, &oldset);
    memset(path_buf, 0, sizeof(path_buf));
    strncpy(path_buf, buddy_path_buf, sizeof(path_buf) - 1);

    stat(path_buf, &buf);
    if(!S_ISREG(buf.st_mode)) {
        upk_alert("cannot exec: %s: %s\n", path_buf, strerror(EPERM));
        return 0;
    }
    if((buf.st_mode & S_IXUSR) != S_IXUSR) {
        upk_alert("cannot exec: %s: %s\n", path_buf, strerror(EACCES));
        return 0;
    }


    if((pid = fork()) == 0) {
        sigemptyset(&sigact.sa_mask);
        sigact.sa_flags = 0;
        sigact.sa_handler = SIG_DFL;

        sigaction(SIGCHLD, &sigact, NULL);
        sigaction(SIGTERM, &sigact, NULL);
        sigaction(SIGINT, &sigact, NULL);
        sigaction(SIGPIPE, &sigact, NULL);


        /* dynamic allocation is mostly safe here; if it fails, the exec will probably fail; discounting
           ulimits */
        // assert((pathp = calloc(1, strlen(path_buf) + 1)));
        // strcpy(pathp, path_buf);

        buddy_setreguid();

        /* TODO: Add support for ulimit contraints on managed processes */


        if(proc_pid)
            upk_notice("executing %s %d\n", path_buf, proc_pid);
        else
            upk_notice("executing %s\n", path_buf);

        upk_debug0("redirecting output\n");
        buddy_setup_fds();

        /* a child may reset these itself, but if a user knows that their service does not; they may use one
           or both identifiers to ensure that all of the services children have exited before considering the 
           service stopped, or perhaps more interesting things */

        setsid();
        setpgid(0, 0);

        /* TODO: Support cgroups on linux (i.e. use cgcreate/cgexec to be used similarly to the pgrp or sid
           above, but in a way that cannot be abandoned by the monitored process; can then use the cgroup
           context to set process limits) */

        memset(buddy_string_buf, 0, sizeof(buddy_string_buf));
        snprintf(buddy_string_buf, sizeof(buddy_string_buf) - 1, "%d", proc_pid);
        sigemptyset(&sigset);
        sigprocmask(SIG_SETMASK, &sigset, NULL);
        errno = 0;
        if(proc_pid) {
            execle(path_buf, path_buf, buddy_string_buf, (char *) NULL, proc_envp);
        } else {
            execle(path_buf, path_buf, (char *) NULL, proc_envp);
        }

        // fprintf(oldstderr, "exec failed for %s %d; This should never happen!: %s\n", path_buf, proc_pid,
        // strerror(errno));
        _exit(errno);
    }
    sigprocmask(SIG_SETMASK, &oldset, NULL);

    return pid;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static                  bool
buddy_start_proc(void)
{
    static sigset_t         sigset, oldset;

    if(proc_pid == 0) {
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGCHLD);
        sigprocmask(SIG_BLOCK, &sigset, &oldset);

        buddy_path("actions/start");

        if(respawn_timestamp + RESPAWN_WINDOW > time(NULL))
            ++rapid_respawn_count;
        else
            rapid_respawn_count = 0;

        if(rapid_respawn_count > MAX_RESPAWN_COUNT) {
            upk_notice("%s's start script respawning too fast, imposing ratelimit for %d seconds\n",
                       buddy_service_name, RESPAWN_RATELIMIT);
            force_ratelimit = true;
            return false;
        }

        proc_pid = buddy_exec_action();
        respawn_timestamp = time(NULL);
        upk_debug0("started child with pid: %d\n", proc_pid);
        buddy_record_state();
        info_ringbuffer = info_ringbuffer->next;

        sigprocmask(SIG_SETMASK, &oldset, NULL);

    }
    return true;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
buddy_stop_proc(void)
{
    static uint8_t          n = 0;

    nanotimeout.tv_sec = 0;
    nanotimeout.tv_nsec = 5000000;                         // 100000000L; /* 1/10th a second; */

    if(proc_pid != 0) {
        buddy_path("actions/stop");
        buddy_exec_action();

        /* XXX: Should this level of insistance that the stop script work be located here, or should it only
           be in buddy_cleanup? */

        /* give child time to terminate normally */
        /* FIXME: should be configurable duration to wait for a child to exit cleanly - JB
         There is already configuration parameters for this, just need to implement in controller and here */
        while(proc_pid && n++ < 10)
            nanosleep(&nanotimeout, NULL);

        /* send signal a TERM directly, in the hopes this will do the trick */
        // if(proc_pid)
        // kill(proc_pid, SIGTERM);

        /* FIXME: again, should be configurable - JB */
        // while(proc_pid && n++ < 20)
        // nanosleep(&nanotimeout, NULL);

        // if(proc_pid)
        // kill(proc_pid, SIGKILL);
    }
    return;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
buddy_reload_proc(void)
{
    if(proc_pid != 0) {
        buddy_path("actions/reload");
        buddy_exec_action();
    }
    return;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
buddy_cust_action(uint32_t act_num)
{
    buddy_path("actions/%02d", act_num);
    buddy_exec_action();
    return;
}


/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
buddy_handle_command(void)
{
    switch (this_command) {
    case UPK_CTRL_SHUTDOWN:
        upk_verbose("command `shutdown' received\n");
        desired_state = BUDDY_STOPPED;
        last_command = this_command;
        buddy_shutdown = SIGHUP;
        return;
    case UPK_CTRL_STATUS_REQ:
        upk_debug1("command `status' received\n");
        last_command = this_command;
        return;
    case UPK_CTRL_ACTION_START:
        upk_verbose("command `start' received\n");
        desired_state = BUDDY_RUNNING;
        last_command = this_command;
        buddy_start_proc();
        return;
    case UPK_CTRL_ACTION_STOP:
        upk_verbose("command `stop' received\n");
        desired_state = BUDDY_STOPPED;
        last_command = this_command;
        buddy_stop_proc();
        return;
    case UPK_CTRL_ACTION_RELOAD:
        upk_verbose("command `reload' received\n");
        last_command = this_command;
        buddy_reload_proc();
        return;
    case UPK_CTRL_ACTION_RUNONCE:
        upk_verbose("command `runonce' received\n");
        desired_state = BUDDY_RANONCE;
        last_command = this_command;
        buddy_start_proc();
        buddy_shutdown = SIGHUP;
        return;
    default:
        break;
    }
    if(this_command >= UPK_CTRL_CUSTOM_ACTION_00 && this_command <= UPK_CTRL_CUSTOM_ACTION_31) {
        upk_verbose("command to invoke action %2d' received\n",
                    (uint32_t) (this_command - UPK_CTRL_CUSTOM_ACTION_00));
        last_command = this_command;
        buddy_cust_action((uint32_t) (this_command - UPK_CTRL_CUSTOM_ACTION_00));
        return;
    } else if(this_command >= UPK_CTRL_SIGNAL_01 && this_command <= UPK_CTRL_SIGNAL_32) {
        upk_verbose("command `signal' (with signal `%d') received\n",
                    (this_command - UPK_CTRL_SIGNAL_01) + 1);
        last_command = this_command;
        kill(proc_pid, (uint32_t) (this_command - UPK_CTRL_SIGNAL_01) + 1);
        return;
    }
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
buddy_setreguid(void)
{
    if(buddy_setuid)
        setreuid(buddy_setuid, buddy_setuid);              /* no point in capturing errors here, because
                                                              there's nothing we can really do; we complain
                                                              in buddy_init() if we think this won't work */
    if(buddy_setgid)
        setregid(buddy_setgid, buddy_setgid);              /* ditto */
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
buddy_setup_fds(void)
{
    freopen(buddy_path("log/stdout"), "a", stdout);
    freopen(buddy_path("log/stderr"), "a", stderr);
    fclose(stdin);
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static void
resolve_symlink(void)
{
    static struct stat      buf;
    static uint8_t          n;

    lstat(buddy_path_buf, &buf);
    n = 0;
    while(S_ISLNK(buf.st_mode)) {
        memset(buddy_string_buf, 0, sizeof(buddy_string_buf));
        readlink(buddy_path_buf, buddy_string_buf, sizeof(buddy_string_buf) - 1);
        strncpy(buddy_path_buf, buddy_string_buf, sizeof(buddy_string_buf) - 1);
        lstat(buddy_path_buf, &buf);
        if(n++ > 32)
            break;
    }
}


/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline           int32_t
read_ctrl(buddy_cmnd_t * buf)
{
    static fd_set           ctrlfdset;

    timeout.tv_sec = BUDDY_SELECT_TIMEOUT_SEC;
    timeout.tv_usec = BUDDY_SELECT_TIMEOUT_USEC;

    highest_fd = buddy_build_fd_set(&ctrlfdset, false);

    if(select(highest_fd + 1, &ctrlfdset, NULL, NULL, &timeout) && FD_ISSET(buddy_ctrlfd, &ctrlfdset)) {
        do {
            errno = 0;
            ncount = read(buddy_ctrlfd, buf, sizeof(*buf));
            if(buddy_disconnect(ncount))
                break;

        } while(ncount < 1);
    }
    return ncount;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline           int32_t
write_ctrl(buddy_info_t * buf)
{
    static fd_set           ctrlfdset;

    timeout.tv_sec = BUDDY_SELECT_TIMEOUT_SEC;
    timeout.tv_usec = BUDDY_SELECT_TIMEOUT_USEC;

    highest_fd = buddy_build_fd_set(&ctrlfdset, false);

    if(select(highest_fd + 1, NULL, &ctrlfdset, NULL, &timeout) && FD_ISSET(buddy_ctrlfd, &ctrlfdset)) {
        do {
            errno = 0;
            ncount = write(buddy_ctrlfd, buf, sizeof(*buf));
            if(buddy_disconnect(ncount))
                break;
        } while(ncount < 1);
    }
    return ncount;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
phone_home_if_appropriate(void)
{
    static uint32_t         n;

    if(buddy_ctrlfd < 0 && ringbuffer_pending > 0) {
        for(n = 0; n < reconnect_retries; n++) {
            if(buddy_shutdown || (ringbuffer_pending >= (ringbuffer_size - (ringbuffer_size / 4)))) {
                if(!buddy_phone_home()) {
                    nanotimeout.tv_sec = 0;
                    nanotimeout.tv_nsec = 500000000L;      /* 1/10th a second; */
                    upk_verbose("Retrying connection to controller\n");
                    nanosleep(&nanotimeout, NULL);
                    continue;
                }
            }
            break;
        }
    }
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static void
buddy_flush_ringbuffer(void)
{
    static sigset_t         sigset, oldset;
    static buddy_cmnd_t     ack, curcmnd;
    static buddy_info_t    *thisp;

    ack = curcmnd = this_command;
    sigfillset(&sigset);

    upk_debug1("flushing ringbuffer; which currently has %d pending msg's \n", ringbuffer_pending);
    phone_home_if_appropriate();

    if(buddy_ctrlfd >= 0) {
        sigprocmask(SIG_BLOCK, &sigset, &oldset);

        thisp = info_ringbuffer;
        do {
            if(info_ringbuffer->populated) {
                info_ringbuffer->remaining = ringbuffer_pending - 1;
                if(write_ctrl(info_ringbuffer) > 0) {
                    if((read_ctrl(&ack)) > 0 && ack == UPK_CTRL_ACK) {
                        buddy_zero_info();
                        info_ringbuffer = info_ringbuffer->next;
                        continue;
                    }
                }
                info_ringbuffer = thisp;
                break;
            }
            info_ringbuffer = info_ringbuffer->next;
        } while(info_ringbuffer != thisp);
        errno = 0;
        buddy_disconnect(0);

        this_command = curcmnd;
        sigprocmask(SIG_SETMASK, &oldset, NULL);
    }
    return;
}
