
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

#include "controller.h"
#include <stdio.h>
#include <errno.h>


static ctrl_sigqueue_meta_t *ctrl_signal_queue;
static upk_conn_handle_meta_t *clients_list_for_cleanup;

void                    controller_packet_callback(upk_conn_handle_meta_t * clients, upk_payload_t *);

/********************************************************************************************************************
 *********************************************************************************************************************/
static void
accept_conn(int32_t listen_sock, upk_conn_handle_meta_t * clients)
{
    int32_t                 conn_fd = -1;
    struct sockaddr_un      c_sa = { 0 };
    socklen_t               c_sa_len = sizeof(c_sa);

    conn_fd = accept(listen_sock, (struct sockaddr *) &c_sa, &c_sa_len);
    if(conn_fd >= 0)
        upk_net_add_socket_handle(clients, conn_fd, controller_packet_callback);
}

/********************************************************************************************************************
 *********************************************************************************************************************/
void
controller_packet_callback(upk_conn_handle_meta_t * clients, upk_payload_t * msg)
{
    upk_conn_handle_t      *handle = clients->thisp;
    upk_svc_desc_meta_t    *svclist = upk_runtime_configuration.svclist;
    upk_packet_t           *reply = NULL;

    switch (msg->type) {
    case UPK_REQ_ACTION:
        upk_debug1("action request received for service %s, type: %s\n", msg->payload.req_action.svc_id, msg->payload.req_action.action);

        /* FIXME: should have a hash table/dictionary for more efficient service lookup */
        UPKLIST_FOREACH(svclist) {
            if(strncmp(svclist->thisp->Name, msg->payload.req_action.svc_id, UPK_MAX_STRING_LEN) == 0) {
                if(strncasecmp(msg->payload.req_action.action, "start", UPK_MAX_STRING_LEN) == 0) {
                    upk_debug1("Sending start request to buddy\n");
                    if(start_buddy_svc(svclist->thisp))
                        reply = upk_create_repl_result(handle, "success", true);
                    else
                        reply = upk_create_repl_result(handle, "failed", false);
                } else if(strncasecmp(msg->payload.req_action.action, "stop", UPK_MAX_STRING_LEN) == 0) {
                    upk_debug1("Sending stop request to buddy\n");
                    if(stop_buddy_svc(svclist->thisp))
                        reply = upk_create_repl_result(handle, "success", true);
                    else
                        reply = upk_create_repl_result(handle, "failed", false);
                } else if(strncasecmp(msg->payload.req_action.action, "reload", UPK_MAX_STRING_LEN) == 0) {
                    upk_debug1("Sending reload request to buddy\n");
                    if(reload_buddy_svc(svclist->thisp))
                        reply = upk_create_repl_result(handle, "success", true);
                    else
                        reply = upk_create_repl_result(handle, "failed", false);
                } else if(strncasecmp(msg->payload.req_action.action, "unconfigure", UPK_MAX_STRING_LEN) == 0) {
                    /* 
                       if(upk_unconfigure_buddy_svc(svclist)) {


                     */
                }

                upk_queue_packet(handle, reply, NULL, NULL);
                upk_pkt_free(reply);
                break;
            }
        }
        break;
    case UPK_REQ_PREAMBLE:
        upk_debug1("negotiating preamble\n");
        int32_t                 clmin, clmax, n, p;

        clmin = msg->payload.req_preamble.min_supported_ver;
        clmax = msg->payload.req_preamble.max_supported_ver;

        if(clmin > UPK_MAX_SUPPORTED_PROTO || clmax < UPK_MIN_SUPPORTED_PROTO) {
            upk_debug1("unsupported version range from client\n");
            break;
        }

        for(n = clmax; (n >= clmin) && !reply; n--)
            for(p = UPK_MAX_SUPPORTED_PROTO; p >= UPK_MIN_SUPPORTED_PROTO && !reply; p--)
                if(n == p)
                    reply = upk_create_repl_preamble(handle, p);

        upk_debug1("negotiated version: %d\n", ((upk_repl_preamble_t *) reply->payload)->best_version);
        upk_debug1("setting clientid: %s\n", msg->payload.req_preamble.client_name);

        strncpy(handle->cl_name, msg->payload.req_preamble.client_name, UPK_MAX_STRING_LEN - 1);

        upk_queue_packet(handle, reply, NULL, NULL);
        upk_pkt_free(reply);
        break;
    case UPK_REQ_DISCONNECT:
        upk_debug1("disconnecting client: `%s'\n", handle->cl_name);

        reply = upk_create_req_disconnect(handle);
        upk_queue_packet(handle, reply, upk_net_shutdown_callback, NULL);
        upk_pkt_free(reply);

        break;
    default:
        upk_debug1("request received for service %s, but unhandled\n");
        break;
    }
    return;
}



/********************************************************************************************************************
  @brief place received signals into a queue to handle later.
  
  Because most signal handling will have fairly significant work to do, all signal handling is done via queuing the
  receipt to the 'ctrl_signal_queue', and processing is deferred to the event loop; specifically the "handle_signals"
  function called first in the event loop.
 ********************************************************************************************************************/
void
sa_sigaction_func(int signal, siginfo_t * siginfo, void *ucontext)
{
    UPKLIST_APPEND(ctrl_signal_queue);
    ctrl_signal_queue->thisp->siginfo = *siginfo;
    ctrl_signal_queue->thisp->signal = signal;
}

/********************************************************************************************************************
  @brief cleanup allocated structures at exit; makes valgrind happy, so I can find more signicant issues 
 ********************************************************************************************************************/
void
ctrl_exit_cleanup(void)
{
    UPKLIST_FREE(ctrl_signal_queue);
    UPKLIST_FREE(clients_list_for_cleanup);
}


/********************************************************************************************************************
  @brief setup signal handlers 
 ********************************************************************************************************************/
static inline void
ctrl_setup_sighandlers(void)
{
    struct sigaction        sigact = {
        .sa_flags = SA_NOCLDSTOP | SA_RESTART | SA_SIGINFO,
        .sa_sigaction = sa_sigaction_func
    };

    sigfillset(&sigact.sa_mask);

    errno = 0;
    if(sigaction(SIGCHLD, &sigact, NULL) != 0)
        upk_fatal("sigchld handler registration failed: %s", strerror(errno));
    errno = 0;
    if(sigaction(SIGTERM, &sigact, NULL) != 0)
        upk_fatal("sigterm handler registration failed: %s", strerror(errno));
    errno = 0;
    if(sigaction(SIGINT, &sigact, NULL) != 0)
        upk_fatal("sigint handler registration failed: %s", strerror(errno));

    /* ignore sigpipe, so socket writes don't abort */
    memset(&sigact, 0, sizeof(sigact));
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = SIG_IGN;

    sigaction(SIGPIPE, &sigact, NULL);
}

/********************************************************************************************************************
  @brief the main event loop.

  This will handle signals, poll buddies, poll clients, publish events, and cleanup sockets.
 ********************************************************************************************************************/
void
event_loop(int32_t listen_sock)
{
    upk_conn_handle_meta_t *clients;
    fd_set                  lfds;
    double                  sel_ival = upk_runtime_configuration.BuddyPollingInterval / 2;
    struct timeval          timeout, timeoutv = { 0, 0 };

#ifdef UPK_CONTROLLER_TERMINATE_FOR_TESTING_AFTER_N
    int                     connections = 0;
#endif

    ctrl_signal_queue = calloc(1, sizeof(*ctrl_signal_queue));
    ctrl_setup_sighandlers();

    clients = upk_net_conn_handle_init(NULL, NULL);
    clients_list_for_cleanup = clients;
    atexit(ctrl_exit_cleanup);


    while(1) {
        handle_signals();
        handle_buddies();

        FD_ZERO(&lfds);
        FD_SET(listen_sock, &lfds);

        timeout = timeoutv;
        if(select(listen_sock + 1, &lfds, NULL, NULL, &timeout) > 0) {
            if(FD_ISSET(listen_sock, &lfds)) {
                upk_debug1("Accepting connection\n");
                accept_conn(listen_sock, clients);
#ifdef UPK_CONTROLLER_TERMINATE_FOR_TESTING_AFTER_N
                connections++;
#endif
            }
        }

        upk_net_event_dispatcher(clients, sel_ival);
        upk_net_flush_closed_sockets(clients);
#ifdef UPK_CONTROLLER_TERMINATE_FOR_TESTING_AFTER_N
        if(n++ > 50 || connections > UPK_CONTROLLER_TERMINATE_FOR_TESTING_AFTER_N)
            break;
#endif
    }
}


/********************************************************************************************************************
  @brief setup controller's socket
 *********************************************************************************************************************/
int32_t
ctrl_sock_setup()
{
    struct sockaddr_un      sa = { 0 };
    int32_t                 sock_fd = -1;

    // int32_t sockopts = SO_PASSCRED;

    (void) unlink((const char *) upk_runtime_configuration.controller_socket);
    strncpy(sa.sun_path, (const char *) upk_runtime_configuration.controller_socket, sizeof(sa.sun_path) - 1);
    sa.sun_family = AF_UNIX;

    sock_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    UPK_FUNC_ASSERT(sock_fd >= 0, UPK_SOCKET_FAILURE);
    UPK_FUNC_ASSERT(fcntl(sock_fd, F_GETFL) >= 0, UPK_SOCKET_FAILURE);

    /* FIXME: abstract the code for the socket a connection is coming from away from here so that platform-dependent methods for read-only, 
       and read-write access can be provided (i.e. on linux, use SO_PASSCRED to determine if the client has permission to write/update
       services, or just read state, or none of the above. On other platforms, SO_PEERCRED and getpeereid can be used for this purpose; and 
       on hosts not supporting credential passing via any mechanism, two distinct sockets should be used, and secured via directory and/or
       file permissions */

    UPK_FUNC_ASSERT_MSG(bind(sock_fd, (struct sockaddr *) &sa, sizeof(sa)) == 0, UPK_SOCKET_FAILURE, "could not bind: %s: %s",
                        upk_runtime_configuration.controller_socket, strerror(errno));
    UPK_FUNC_ASSERT_MSG(listen(sock_fd, SOMAXCONN) == 0, UPK_SOCKET_FAILURE, "could not listen: %s: %s",
                        upk_runtime_configuration.controller_socket, strerror(errno));

    IF_UPK_ERROR {
        sock_fd = -1;
    }

    return sock_fd;
}

/********************************************************************************************************************
  @brief deal with signals pending action in signal_queue.

  Block all signals during copy to avoid annoying race conditions.
 ********************************************************************************************************************/
void
handle_signals(void)
{
    sigset_t                sigset, oldset;
    int                     waitstatus;
    ctrl_sigqueue_t         signal_node;

    while(ctrl_signal_queue->count > 0) {
        /* block signals, copy and remove the current element, and then unblock signals; this allows other signals to be enqueued
           while we deal with the current signal */

        sigfillset(&sigset);
        sigprocmask(SIG_BLOCK, &sigset, &oldset);

        UPKLIST_HEAD(ctrl_signal_queue);
        signal_node = *ctrl_signal_queue->thisp;
        UPKLIST_UNLINK(ctrl_signal_queue);

        sigprocmask(SIG_SETMASK, &oldset, NULL);

        switch (signal_node.signal) {
        case SIGCHLD:
            waitpid(signal_node.siginfo.si_pid, &waitstatus, 0);
            /* FIXME: update data-store with rumors of buddy's death (do not exaggerate) */
            break;
        case SIGINT:
        case SIGTERM:
            /* FIXME: defer exit until all other signals are handles do data-store is updated for any known buddy-exits */
            exit(0);
        }
    }

}
