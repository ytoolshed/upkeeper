#include "controller.h"
#include <stdio.h>
#include <errno.h>

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static int
build_fd_set(fd_set * socks, int32_t sock, client_fdlist_head_t * clients)
{
    uint32_t                highest_fd = sock;

    FD_ZERO(socks);
    FD_SET(sock, socks);

    UPKLIST_FOREACH(clients) {
        if(clients->thisp->fd > 0 && fcntl(clients->thisp->fd, F_GETFL) >= 0) {
            FD_SET(clients->thisp->fd, socks);
            highest_fd = (clients->thisp->fd > highest_fd) ? clients->thisp->fd : highest_fd;
        }
    }
    return highest_fd;
}

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
static void
accept_conn(int32_t listen_sock, client_fdlist_head_t * clients)
{
    int32_t                 conn_fd = -1;
    struct sockaddr_un      c_sa = { 0 };
    socklen_t               c_sa_len = sizeof(c_sa);
    int                     sockopts = 0;

    conn_fd = accept(listen_sock, (struct sockaddr *) &c_sa, &c_sa_len);
    if(conn_fd > 0 && (sockopts = fcntl(conn_fd, F_GETFL))) {
        fcntl(conn_fd, F_SETFL, sockopts | O_NONBLOCK);
        fcntl(conn_fd, F_SETFD, FD_CLOEXEC);
        UPKLIST_APPEND(clients);
        clients->thisp->fd = conn_fd;
        upk_debug1("Accepting connection on fd:%d\n", conn_fd);
    }
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
disconnect_client(client_fdlist_head_t * clients)
{
    shutdown(clients->thisp->fd, SHUT_RDWR);
    close(clients->thisp->fd);
    UPKLIST_UNLINK(clients);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static inline void
controller_packet_callback(upk_packet_t * pkt, client_fdlist_head_t *clients)
{
    upk_svc_desc_head_t *svclist = upk_runtime_configuration.svclist;
    upk_action_req_t *action = NULL;
    upk_packet_t *reply = NULL;
    upk_pkt_buf_t *buf;
    size_t n_written = 0;
    int sockopts = 0;
    upk_protocol_handle_t handle = { 0 };

    //upk_req_preamble_t *preamble = NULL;

    switch (pkt->pkttype) {
    case PKT_REQUEST:
        switch (((upk_req_preamble_t *) pkt->payload)->msgtype) {
        case UPK_REQ_ACTION:
            action = pkt->payload;

            upk_debug1("action request received for service %s, type: %s\n", action->svc_id, action->action);
            UPKLIST_FOREACH(svclist) {
                if(strncmp(svclist->thisp->Name, action->svc_id, UPK_MAX_STRING_LEN) == 0) {
                    if(strncasecmp(action->action, "start", UPK_MAX_STRING_LEN) == 0) {
                        upk_debug1("Sending start request to buddy\n");
                        if( start_buddy_svc(svclist->thisp) ) 
                            reply = upk_create_result_repl(&handle, "success", true);
                        else
                            reply = upk_create_result_repl(&handle, "failed", false);
                    }
                    else if(strncasecmp(action->action, "stop", UPK_MAX_STRING_LEN) == 0) {
                        upk_debug1("Sending stop request to buddy\n");
                        if( stop_buddy_svc(svclist->thisp) )
                            reply = upk_create_result_repl(&handle, "success", true);
                        else
                            reply = upk_create_result_repl(&handle, "failed", false);
                    }
                    else if(strncasecmp(action->action, "reload", UPK_MAX_STRING_LEN) == 0) {
                        upk_debug1("Sending reload request to buddy\n");
                        if( reload_buddy_svc(svclist->thisp) )
                            reply = upk_create_result_repl(&handle, "success", true);
                        else
                            reply = upk_create_result_repl(&handle, "failed", false);
                    }

                    buf = upk_serialize_packet(reply);

                    /* FIXME: cannot block */
                    sockopts = fcntl(clients->thisp->fd, F_GETFL);
                    fcntl(clients->thisp->fd, F_SETFL, sockopts ^ O_NONBLOCK);

                    errno=0;
                    n_written = write(clients->thisp->fd, buf, 20 + reply->payload_len);
                    if(errno)
                        upk_debug1("%s\n", strerror(errno));

                    fcntl(clients->thisp->fd, F_SETFL, sockopts | O_NONBLOCK);
                    free(buf);
                    upk_pkt_free(reply);
                    disconnect_client(clients);
                    break;
                }
            }
            break;
        case UPK_REQ_PREAMBLE:
            upk_debug1("Preamble request received, negotiating version: %d", UPK_MIN_SUPPORTED_PROTO);
            break;
        default:
            upk_debug1("request received for service %s, but unhandled\n");
            break;

        }
        break;
    default:
        upk_alert("invalid packet type\n");
        break;
    }
    return;
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_read_packet(client_fdlist_head_t * clients)
{
    int32_t                 n_read;
    client_fdlist_t        *client = clients->thisp;
    uint32_t                payload_len, version_id;
    upk_packet_t           *pkt;

    /* read the 16 header bytes, non-blocking to prevent a rogue client from stopping up the works,
       so we have to nibble */

    if(client->n_remaining_read == 0 && client->n_hdr_bytes_read < 16) {
        errno = 0;
        n_read = read(client->fd, client->readbuf, 16 - client->n_hdr_bytes_read);
        if(n_read < 1 && !(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
            disconnect_client(clients);
            return;
        }
        client->n_hdr_bytes_read += n_read;
    }

    if(client->n_remaining_read == 0 && client->n_hdr_bytes_read == 16) {
        payload_len = ntohl(*((uint32_t *) client->readbuf));
        version_id = ntohl(*((uint32_t *) client->readbuf+1));
        client->n_hdr_bytes_read = 0;

        if(! (version_id >= UPK_MIN_SUPPORTED_PROTO && version_id <= UPK_MAX_SUPPORTED_PROTO) ) {
            upk_error("received invalid packet: invalid version_id: %d\n", version_id);
            disconnect_client(clients);
            return;
        }
        if(payload_len > UPK_MAX_PACKET_SIZE) {
            upk_error("received invalid packet: invalid payload_len: %d\n", payload_len);
            disconnect_client(clients);
            return;
        }
        client->n_remaining_read = payload_len + 4;
    }

    if(client->n_remaining_read > 0) {
        n_read = read(client->fd, client->readbuf + 16, client->n_remaining_read);
        if(n_read < 1 && !(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
            disconnect_client(clients);
            return;
        }
        client->n_remaining_read -= n_read;

        if(client->n_remaining_read == 0) {
            pkt = upk_deserialize_packet(client->readbuf);
            controller_packet_callback(pkt, clients);
        }
    }
}


/* *******************************************************************************************************************
   ****************************************************************************************************************** */
void
event_loop(int32_t listen_sock)
{
    client_fdlist_head_t   *clients;
    fd_set                  socks;
    int32_t                 highest_fd = listen_sock;
    struct timeval          timeout = {.tv_sec = 0,.tv_usec = 10000 };
    clients = calloc(1, sizeof(*clients));

    FD_ZERO(&socks);

    while(1) {
        highest_fd = build_fd_set(&socks, listen_sock, clients);
        handle_signals();
        handle_buddies();

        timeout.tv_sec = (int) upk_runtime_configuration.BuddyPollingInterval;
        timeout.tv_usec = (int) ( (upk_runtime_configuration.BuddyPollingInterval - (double) ((int) upk_runtime_configuration.BuddyPollingInterval)) * 1000000 ) ;

        if(select(highest_fd + 1, &socks, NULL, NULL, &timeout) > 0) {
            if(FD_ISSET(listen_sock, &socks)) {
                upk_debug1("Accepting connection\n");
                accept_conn(listen_sock, clients);
            }

            UPKLIST_FOREACH(clients) {
                if(clients->thisp->fd > 0 && fcntl(clients->thisp->fd, F_GETFL)) {
                    if(FD_ISSET(clients->thisp->fd, &socks)) {
                        upk_read_packet(clients);
                    }
                } else {
                    disconnect_client(clients);
                }
            }
        }
    }
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
