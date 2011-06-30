#include "controller.h"
#include <stdio.h>
#include <errno.h>

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static int
build_fd_set(fd_set * socks, int32_t sock, void *client_ll)
{
    UPKLIST_METANODE(client_fdlist_t, clients) = client_ll;
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
 * ****************************************************************************************************************** */
static void
accept_conn(int32_t listen_sock, void *client_ll)
{
    UPKLIST_METANODE(client_fdlist_t, clients) = client_ll;
    int32_t                 conn_fd = -1;
    struct sockaddr_un      c_sa = { 0 };
    socklen_t               c_sa_len = sizeof(c_sa);
    int                     sockopts = 0;

    conn_fd = accept(listen_sock, (struct sockaddr *) &c_sa, &c_sa_len);
    if(conn_fd > 0 && (sockopts = fcntl(conn_fd, F_GETFL))) {
        fcntl(conn_fd, F_SETFL, sockopts | O_NONBLOCK);
        UPKLIST_APPEND(clients);
        clients->thisp->fd = conn_fd;
    }
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
static inline void
disconnect_client(void *client_ll)
{
    UPKLIST_METANODE(client_fdlist_t, clients) = client_ll;
    close(clients->thisp->fd);
    UPKLIST_UNLINK(clients);
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
void
event_loop(int32_t listen_sock)
{
    UPKLIST_INIT(client_fdlist_t, clients);
    fd_set                  socks;
    int32_t                 highest_fd = listen_sock;
    struct timeval          timeout = {.tv_sec = 0,.tv_usec = 10000 };
    // uint32_t n = 0;

    FD_ZERO(&socks);

    while(1) {
        highest_fd = build_fd_set(&socks, listen_sock, clients);
        handle_signals();
        handle_buddies();

        timeout.tv_sec = 0;
        timeout.tv_usec = 10000; 

        if(select(highest_fd + 1, &socks, NULL, NULL, &timeout) > 0) {
            if(FD_ISSET(listen_sock, &socks))
                accept_conn(listen_sock, clients);

            UPKLIST_FOREACH(clients) {
                if(clients->thisp->fd > 0 && fcntl(clients->thisp->fd, F_GETFL)) {
                    if(FD_ISSET(clients->thisp->fd, &socks)) {
                        switch (handle_client(clients->thisp->fd)) {
                        case UPK_CLIENT_DISCONNECT:
                            disconnect_client(clients);
                            break;
                        }
                    }
                } else {
                    disconnect_client(clients);
                }
            }
        }
    }
}


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
int32_t
ctrl_sock_setup()
{
    struct sockaddr_un      sa = { 0 };
    int32_t                 sock_fd = -1;

    // int32_t sockopts = SO_PASSCRED;

    UPK_ERR_INIT;


    (void) unlink((const char *) upk_ctrl_config->controller_socket);
    strncpy(sa.sun_path, (const char *) upk_ctrl_config->controller_socket, sizeof(sa.sun_path) - 1);
    sa.sun_family = AF_UNIX;

    sock_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    UPK_FUNC_ASSERT(sock_fd >= 0, UPK_SOCKET_FAILURE);
    UPK_FUNC_ASSERT(fcntl(sock_fd, F_GETFL) >= 0, UPK_SOCKET_FAILURE);

    /* setsockopt(sock_fd, SOL_SOCKET, SO_PASSCRED, &sockopts, sizeof(sockopts)); */

    UPK_FUNC_ASSERT_MSG(bind(sock_fd, (struct sockaddr *) &sa, sizeof(sa)) == 0, UPK_SOCKET_FAILURE,
                        "could not bind: %s: %s", upk_ctrl_config->controller_socket, strerror(errno));
    UPK_FUNC_ASSERT_MSG(listen(sock_fd, SOMAXCONN) == 0, UPK_SOCKET_FAILURE, "could not listen: %s: %s",
                        upk_ctrl_config->controller_socket, strerror(errno));

    IF_UPK_ERROR {
        sock_fd = -1;
    }

    return sock_fd;
}

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
void
handle_signals(void)
{
    return;
}
