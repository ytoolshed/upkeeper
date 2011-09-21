/* ***************************************************************************
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

void                    controller_packet_callback(upk_conn_handle_meta_t * clients, upk_payload_t *);

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
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

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
controller_packet_callback(upk_conn_handle_meta_t * clients, upk_payload_t * msg)
{
    upk_conn_handle_t      *handle = clients->thisp;
    upk_svc_desc_meta_t    *svclist = upk_runtime_configuration.svclist;
    upk_packet_t           *reply = NULL;

    switch (msg->type) {
    case UPK_REQ_ACTION:
        upk_debug1("action request received for service %s, type: %s\n", msg->payload.req_action.svc_id,
                   msg->payload.req_action.action);
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
                }

                upk_queue_packet(handle, reply, NULL, NULL);
                upk_pkt_free(reply);
                break;
            }
        }
        break;
    case UPK_REQ_PREAMBLE:
        upk_debug1("negotiating preamble\n");
        int32_t clmin, clmax, n, p;
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

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
void
event_loop(int32_t listen_sock)
{
    upk_conn_handle_meta_t *clients;
    fd_set                  lfds;
    double                  sel_ival = upk_runtime_configuration.BuddyPollingInterval / 2;
    struct timeval          timeout, timeoutv = { 0, 0 };
    int n = 0;


    clients = upk_net_conn_handle_init(NULL, NULL);

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
            }
        }

        upk_net_event_dispatcher(clients, sel_ival);
        upk_net_flush_closed_sockets(clients);
        if(n++ > 50)
            break;
    }
    UPKLIST_FREE(clients);
}


/* *******************************************************************************************************************
   ****************************************************************************************************************** */
int32_t
ctrl_sock_setup()
{
    struct sockaddr_un      sa = { 0 };
    int32_t                 sock_fd = -1;

    // int32_t sockopts = SO_PASSCRED;

    UPK_ERR_INIT;


    (void) unlink((const char *) upk_runtime_configuration.controller_socket);
    strncpy(sa.sun_path, (const char *) upk_runtime_configuration.controller_socket, sizeof(sa.sun_path) - 1);
    sa.sun_family = AF_UNIX;

    sock_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    UPK_FUNC_ASSERT(sock_fd >= 0, UPK_SOCKET_FAILURE);
    UPK_FUNC_ASSERT(fcntl(sock_fd, F_GETFL) >= 0, UPK_SOCKET_FAILURE);

    /* setsockopt(sock_fd, SOL_SOCKET, SO_PASSCRED, &sockopts, sizeof(sockopts)); */

    UPK_FUNC_ASSERT_MSG(bind(sock_fd, (struct sockaddr *) &sa, sizeof(sa)) == 0, UPK_SOCKET_FAILURE,
                        "could not bind: %s: %s", upk_runtime_configuration.controller_socket, strerror(errno));
    UPK_FUNC_ASSERT_MSG(listen(sock_fd, SOMAXCONN) == 0, UPK_SOCKET_FAILURE, "could not listen: %s: %s",
                        upk_runtime_configuration.controller_socket, strerror(errno));

    IF_UPK_ERROR {
        sock_fd = -1;
    }

    return sock_fd;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
void
handle_signals(void)
{
    return;
}
