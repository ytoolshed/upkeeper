#include "buddy.h"
#include <assert.h>
#include <stdarg.h>

bool                    chld_terminated;
int                     buddy_shutdown;
siginfo_t               proc_siginfo;
int                     proc_waitstatus;
pid_t                   proc_pid;
buddy_commands_t        last_command;
int32_t                 verbose;
uid_t                   buddy_uid;
gid_t                   buddy_gid;

size_t                  ringbuffer_size;
buddy_info_t           *info_ringbuffer;

int                     buddy_sock_fd;
int                     buddy_ctrl_fds[BUDDY_MAX_CONNECTIONS];
struct sockaddr_un      buddy_sockaddr;

char                    buddy_path_buf[BUDDY_MAX_PATH_LEN];
char                    buddy_fmt_buf[BUDDY_MAX_PATH_LEN];
char                   *buddy_service_name;
char                    buddy_root_path[] = DEFAULT_BUDDY_ROOT;

static void             free_ringbuffer(void);

static inline char     *
buddy_path(const char *suffixfmt, ...)
{
    va_list                 ap;

    snprintf(buddy_fmt_buf, BUDDY_MAX_STRING_LEN, "%s/%s/%s", buddy_root_path, buddy_service_name, suffixfmt);

    va_start(ap, suffixfmt);
    vsnprintf(buddy_path_buf, BUDDY_MAX_STRING_LEN, buddy_fmt_buf, ap);
    va_end(ap);

    return buddy_path_buf;
}

/* ********************************************************************************************************************
 * ******************************************************************************************************************* */
static void
init_socket(void)
{
    (void) unlink(buddy_path("buddy.sock"));

    strncpy(buddy_sockaddr.sun_path, (const char *) buddy_path("buddy.sock"), sizeof(buddy_sockaddr.sun_path) - 1);
    buddy_sockaddr.sun_family = AF_UNIX;

    buddy_sock_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    assert(buddy_sock_fd >= 0);
    assert(fcntl(buddy_sock_fd, F_GETFL) >= 0);

    assert(bind(buddy_sock_fd, (struct sockaddr *) &buddy_sockaddr, sizeof(buddy_sockaddr)) == 0);
    assert(listen(buddy_sock_fd, SOMAXCONN) == 0);
}

/* ********************************************************************************************************************
 * ******************************************************************************************************************* */
static void
flush_ringbuffer(void)
{
    return;
}

/* ********************************************************************************************************************
 * ******************************************************************************************************************* */
void
buddy_cleanup(void)
{
    flush_ringbuffer();
    free_ringbuffer();
    unlink(buddy_path("buddy.sock"));
    if(buddy_service_name)
        free(buddy_service_name);
}

/* ********************************************************************************************************************
 * ******************************************************************************************************************* */
void
sa_sigaction_func(int signal, siginfo_t * siginfo, void *ucontext)
{
    size_t                  n;

    switch (signal) {
    case SIGCHLD:
        if(siginfo->si_pid != proc_pid)
            return;
        (void) waitpid(siginfo->si_pid, &proc_waitstatus, 0);
        info_ringbuffer->service_pid = proc_pid;
        info_ringbuffer->command = last_command;
        for(n = 0; n < sizeof(proc_siginfo); n++)
            *((char *) &info_ringbuffer->siginfo + n) = *((char *) siginfo + n);    /* memcpy isn't necessarily
                                                                                     * reentrant; doing this with
                                                                                     * signals blocked should be safe
                                                                                     * (no risk of stepping on another
                                                                                     * memcpy already in progress) */
        info_ringbuffer->wait_status = proc_waitstatus;
        info_ringbuffer->timestamp = time(NULL);

        info_ringbuffer = info_ringbuffer->next;
        chld_terminated = true;
        break;
    default:
        buddy_shutdown = signal;
        break;
    }
    return;
}

/* ********************************************************************************************************************
 * ******************************************************************************************************************* */
static void
init_ringbuffer(void)
{
    uint32_t                n = 0;
    buddy_info_t           *first_node = NULL;

    first_node = calloc(1, sizeof(*first_node));
    info_ringbuffer = first_node;

    for(n = 1; n < ringbuffer_size; n++) {
        info_ringbuffer->next = calloc(1, sizeof(*info_ringbuffer->next));
        info_ringbuffer = info_ringbuffer->next;
    }
    info_ringbuffer->next = first_node;

    return;
}

/* ********************************************************************************************************************
 * ******************************************************************************************************************* */
static void
free_ringbuffer(void)
{
    uint32_t                n = 0;
    buddy_info_t           *nodep = NULL;

    for(n = 0; n < ringbuffer_size; n++) {
        nodep = info_ringbuffer->next;
        free(info_ringbuffer);
        info_ringbuffer = nodep;
    }
    return;
}


/* ********************************************************************************************************************
 * ******************************************************************************************************************* */
void
buddy_init(void)
{
    sigset_t                sigset;
    struct sigaction        sigact;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGCHLD);

    sigact.sa_sigaction = sa_sigaction_func;
    sigfillset(&sigact.sa_mask);
    sigact.sa_flags = SA_NOCLDSTOP | SA_RESTART;

    sigaction(SIGCHLD, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGINT, &sigact, NULL);

    init_ringbuffer();
    init_socket();
}

/* ********************************************************************************************************************
 * ******************************************************************************************************************* */
static void
buddy_accept_conn(int32_t listen_sock, int ctrl_conn[BUDDY_MAX_CONNECTIONS])
{
    int32_t                 conn_fd = -1;
    struct sockaddr_un      c_sa = { 0 };
    socklen_t               c_sa_len = sizeof(c_sa);
    int                     sockopts = 0, n = 0;


    conn_fd = accept(listen_sock, (struct sockaddr *) &c_sa, &c_sa_len);
    if(conn_fd > 0 && (sockopts = fcntl(conn_fd, F_GETFL))) {
        fcntl(conn_fd, F_SETFL, sockopts | O_NONBLOCK);
        for(n = 0; n < BUDDY_MAX_CONNECTIONS; n++) {
            if(ctrl_conn[n] == -1) {
                ctrl_conn[n] = conn_fd;
                break;
            }
        }
    }
}

/* ********************************************************************************************************************
 * ******************************************************************************************************************* */
static int
buddy_build_fd_set(fd_set * socks, int32_t sock, int ctrl_conn[BUDDY_MAX_CONNECTIONS])
{
    uint32_t                highest_fd = sock, n = 0;

    FD_ZERO(socks);
    FD_SET(sock, socks);

    for(n = 0; n < BUDDY_MAX_CONNECTIONS; n++) {
        if(ctrl_conn[n] > 0 && fcntl(ctrl_conn[n], F_GETFL) >= 0) {
            FD_SET(ctrl_conn[n], socks);
            highest_fd = (ctrl_conn[n] > highest_fd) ? ctrl_conn[n] : highest_fd;
        }
    }
    return highest_fd;
}

/* ********************************************************************************************************************
 * ******************************************************************************************************************* */
static void
buddy_handle_signals(void)
{
    return;
}


/* ********************************************************************************************************************
 * ******************************************************************************************************************* */
int32_t
buddy_event_loop(void)
{
    fd_set                  socks;
    int                     highest_fd = 0;
    struct timeval          timeout = {.tv_sec = 0, .tv_usec = 20000 };

    while(1) {
        buddy_handle_signals();
        timeout.tv_sec = 0;
        timeout.tv_usec = 20000;

        highest_fd = buddy_build_fd_set(&socks, buddy_sock_fd, buddy_ctrl_fds);
        if(select(highest_fd + 1, &socks, NULL, NULL, &timeout) > 0) {
            if(FD_ISSET(buddy_sock_fd, &socks))
                buddy_accept_conn(buddy_sock_fd, buddy_ctrl_fds);
        }
        if(buddy_shutdown > 0)
            return buddy_shutdown;

    }
    return 0;
}
