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

#include "upk_include.h"

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static inline int
upk_build_fd_sets(fd_set * readfds, fd_set * writefds, fd_set * exceptfds, upk_conn_handle_meta_t * handles)
{
    uint32_t                nfds = 0;

    FD_ZERO(readfds);
    FD_ZERO(writefds);
    FD_ZERO(exceptfds);

    UPKLIST_FOREACH(handles) {
        if(handles->thisp->fd > 0 && fcntl(handles->thisp->fd, F_GETFL) >= 0) {
            FD_SET(handles->thisp->fd, readfds);
            FD_SET(handles->thisp->fd, writefds);
            FD_SET(handles->thisp->fd, exceptfds);
            nfds = (handles->thisp->fd > nfds) ? handles->thisp->fd : nfds;
        }
    }
    return nfds;
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static void
upk_partitioned_userdata_free(void * ptr)
{
    upklist_userdata_state_partition_t *data = ptr;
    if(!data)
        return;

    if(data->userdata && data->userdata_free_func)
        data->userdata_free_func(data->userdata);

    UPKLIST_FREE(data->gstate->callback_stack);

    free(data->gstate);
    free(ptr);
}


/* *******************************************************************************************************************
   ******************************************************************************************************************* */
upk_conn_handle_meta_t *
upk_net_conn_handle_init(void *userdata, void (*userdata_free_func)(void *ptr))
{
    upk_conn_handle_meta_t *handles = calloc(1, sizeof(*handles));
    upklist_userdata_state_partition_t *data = calloc(1, sizeof(*data));

    data->gstate = calloc(1, sizeof(*data->gstate));
    data->userdata = userdata;
    data->userdata_free_func = userdata_free_func;

    handles->userdata = data;
    handles->userdata_free_func = upk_partitioned_userdata_free;
    return handles;
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
upk_net_gstate_t *
upk_net_get_gstate(upk_conn_handle_meta_t * handles)
{
    upklist_userdata_state_partition_t *data = handles->userdata;
    return data->gstate;
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void *
upk_net_get_userdata(upk_conn_handle_meta_t * handles)
{
    upklist_userdata_state_partition_t *data = handles->userdata;
    return data->userdata;
}


/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_net_add_socket_handle(upk_conn_handle_meta_t * handles, int fd, upk_net_callback_t pkt_callback)
{
    int                     sockopts = 0;

    if(fd > 0 && (sockopts = fcntl(fd, F_GETFL))) {
        fcntl(fd, F_SETFL, sockopts | O_NONBLOCK);
        fcntl(fd, F_SETFD, FD_CLOEXEC);

        UPKLIST_APPEND(handles);
        handles->thisp->userdata = upk_net_get_userdata(handles);
        handles->thisp->gstate = upk_net_get_gstate(handles);
        handles->thisp->fd = fd;
        handles->thisp->after_read_callback = pkt_callback;
    }
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_net_flush_closed_sockets(upk_conn_handle_meta_t * handles)
{
    UPKLIST_FOREACH(handles) {
        if(handles->thisp->fd < 0) {
            UPKLIST_UNLINK(handles);
        }
    }
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_net_event_dispatcher(upk_conn_handle_meta_t * handles, double sel_ival)
{
    fd_set                  readfds, writefds, exceptfds, *writefds_p = NULL;
    int                     nfds, ready;
    sigset_t                origmask, sigmask;
    upk_net_gstate_t       *gstate = upk_net_get_gstate(handles);
    upk_net_cb_stk_meta_t  *callbacks = gstate->callback_stack;
    struct timeval          timeout = upk_double_to_timeval(sel_ival);

    if(callbacks && callbacks->head && callbacks->head->net_dispatch_pre) callbacks->head->net_dispatch_pre(handles, NULL);

    nfds = upk_build_fd_sets(&readfds, &writefds, &exceptfds, handles);
    writefds_p = (gstate->pending_writeq > 0) ? &writefds : NULL;

    sigfillset(&sigmask);
    sigprocmask(SIG_SETMASK, &sigmask, &origmask);
    ready = select(nfds + 1, &readfds, writefds_p, NULL, &timeout);
    sigprocmask(SIG_SETMASK, &origmask, NULL);

    if(ready > 0) {
        UPKLIST_FOREACH(handles) {
            if(callbacks && callbacks->head && callbacks->head->net_dispatch_foreach) callbacks->head->net_dispatch_foreach(handles, NULL);
            if(handles->thisp->fd >= 0 && fcntl(handles->thisp->fd, F_GETFL)) {
                if(FD_ISSET(handles->thisp->fd, &readfds))
                    upk_read_packets(handles);
                if(writefds_p && FD_ISSET(handles->thisp->fd, &writefds))
                    upk_write_packets(handles);
            } else {
                upk_disconnect_handle(handles);
                continue;
            }
        }
    }
    if(callbacks && callbacks->head && callbacks->head->net_dispatch_post) callbacks->head->net_dispatch_post(handles, NULL);
}


/* *******************************************************************************************************************
   ******************************************************************************************************************* */
int
upk_net_block_until_msg(upk_conn_handle_meta_t * handles, double sel_ival, struct timeval *timeout)
{
    struct timeval          start_time = { 0 };
    struct timeval          cur_time = { 0 };
    upk_conn_handle_t      *handle = handles->thisp;


    gettimeofday(&start_time, NULL);
    memset(&handle->last_pkt_data, 0, sizeof(handle->last_pkt_data));

    while(handle->fd >= 0) {
        upk_net_event_dispatcher(handles, sel_ival);
        if(handle->last_pkt_data.type)
            return handle->last_pkt_data.type;

        gettimeofday(&cur_time, NULL);
        if(timeout && (cur_time.tv_sec >= start_time.tv_sec + timeout->tv_sec)) {
            if(timeout && (cur_time.tv_usec >= start_time.tv_usec + timeout->tv_usec))
                return -1;
        }
    }
    return handle->fd;
}


/* *******************************************************************************************************************
   ******************************************************************************************************************* */
static void
upk_call_received_packet_callbacks(upk_conn_handle_meta_t * handles, upk_packet_t * pkt)
{
    upk_payload_t          *data = NULL;
    upk_conn_handle_t      *handle = NULL;
    upk_net_gstate_t       *gstate = upk_net_get_gstate(handles);
    upk_net_cb_stk_meta_t  *callbacks = gstate->callback_stack;

    if(!handles || (handles && !handles->thisp))
        return;

    handle = handles->thisp;
    data = &handle->last_pkt_data;
    memset(data, 0, sizeof(*data));

    data->type = upk_get_msgtype(pkt);
    memcpy(&data->payload, pkt->payload, upk_get_msgsize(data->type));

    if(handle->after_read_callback) 
        handle->after_read_callback(handles, data);
    if(callbacks && callbacks->head && callbacks->head->msg_handlers[UPK_MSGTYPE_IDX(data->type)])
        callbacks->head->msg_handlers[UPK_MSGTYPE_IDX(data->type)](handles, data);
}


/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_disconnect_handle(upk_conn_handle_meta_t * handles)
{
    if(handles && handles->thisp) {
        shutdown(handles->thisp->fd, SHUT_RDWR);
        close(handles->thisp->fd);
        handles->thisp->fd = -2;
    }
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_net_shutdown_callback(upk_conn_handle_meta_t *handles, upk_payload_t *msg)
{
    upk_disconnect_handle(handles);
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_queue_packet(upk_conn_handle_t * handle, upk_packet_t * pkt, upk_net_callback_t after_write_callback,
                 upk_net_callback_t set_after_read_callback)
{
    upk_pkt_buf_t          *bufp = NULL;
    upk_netmsg_queue_t     *msgp = NULL;
    upk_net_gstate_t       *gstate = handle->gstate;

    UPKLIST_APPEND((&handle->writeq));
    gstate->pending_writeq++;
    msgp = handle->writeq.thisp;

    msgp->after_write_callback = after_write_callback;
    msgp->set_after_read_callback = set_after_read_callback;
    msgp->msg_len = UPK_PACKET_HEADER_LEN + pkt->payload_len + UPK_PACKET_FOOTER_LEN;

    bufp = upk_serialize_packet(pkt);
    memcpy(msgp->msg, bufp, msgp->msg_len);

    free(bufp);
}


/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_write_packets(upk_conn_handle_meta_t * handles)
{
    size_t                  n_write = 0;
    upk_netmsg_queue_t     *msgp = NULL;
    upk_conn_handle_t      *handle = handles->thisp;
    upk_net_gstate_t       *gstate = upk_net_get_gstate(handles);

    if(handle->fd < 0)
        return;

    if(handle->writeq.count < 1 || !handle->writeq.thisp)
        return;

    msgp = handle->writeq.thisp;

    if((msgp->msg_len - msgp->n_bytes_written) > 0) {
        n_write = write(handle->fd, msgp->msg + msgp->n_bytes_written, msgp->msg_len - msgp->n_bytes_written);
        if(n_write < 1 && !(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
            upk_disconnect_handle(handles);
            return;
        }
        if(n_write < 0)
            n_write = 0;

        msgp->n_bytes_written += n_write;

        if(msgp->n_bytes_written == msgp->msg_len) {
            if(msgp->after_write_callback)
                msgp->after_write_callback(handles, NULL);
            if(msgp->set_after_read_callback)
                handle->after_read_callback = msgp->set_after_read_callback;
            UPKLIST_UNLINK((&handle->writeq));
            gstate->pending_writeq--;
        }
    }
}

/* *******************************************************************************************************************
   ******************************************************************************************************************* */
void
upk_read_packets(upk_conn_handle_meta_t * handles)
{
    size_t                  n_read = 0;
    upk_conn_handle_t      *handle = NULL;
    uint32_t                payload_len, version_id;
    upk_packet_t           *pkt;

    if(!handles || (handles && !handles->thisp))
        return;

    handle = handles->thisp;

    if(handle->fd < 0)
        return;

    /* read the 16 header bytes, non-blocking to prevent a rogue handle from stopping up the works, so we have to
       nibble */

    if(handle->n_remaining_read == 0 && handle->n_hdr_bytes_read < UPK_PACKET_HEADER_LEN) {
        errno = 0;
        n_read = read(handle->fd, handle->readbuf, UPK_PACKET_HEADER_LEN - handle->n_hdr_bytes_read);
        if(n_read < 1 && !(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
            upk_disconnect_handle(handles);
            return;
        }
        if(n_read < 0)
            n_read = 0;
        handle->n_hdr_bytes_read += n_read;
    }

    if(handle->n_remaining_read == 0 && handle->n_hdr_bytes_read == UPK_PACKET_HEADER_LEN) {
        payload_len = ntohl(*((uint32_t *) handle->readbuf));
        version_id = ntohl(*((uint32_t *) handle->readbuf + 1));
        handle->n_hdr_bytes_read = 0;

        if(!(version_id >= UPK_MIN_SUPPORTED_PROTO && version_id <= UPK_MAX_SUPPORTED_PROTO)) {
            upk_error("received invalid packet: invalid version_id: %d\n", version_id);
            upk_disconnect_handle(handles);
            return;
        }
        if(payload_len > UPK_MAX_PACKET_SIZE) {
            upk_error("received invalid packet: invalid payload_len: %d\n", payload_len);
            upk_disconnect_handle(handles);
            return;
        }
        handle->n_remaining_read = payload_len + UPK_PACKET_FOOTER_LEN;
        upk_debug1("reading packet of length %d\n", handle->n_remaining_read);
    }

    if(handle->n_remaining_read > 0) {
        n_read = read(handle->fd, handle->readbuf + UPK_PACKET_HEADER_LEN, handle->n_remaining_read);
        if(n_read < 1 && !(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
            upk_disconnect_handle(handles);
            return;
        }
        if(n_read < 0)
            n_read = 0;

        handle->n_remaining_read -= n_read;
        upk_debug1("read %d bytes of packet (%d remaining)\n", n_read, handle->n_remaining_read);

        if(handle->n_remaining_read == 0) {
            pkt = upk_deserialize_packet(handle->readbuf);
            upk_call_received_packet_callbacks(handles, pkt);
            upk_pkt_free(pkt);
            memset(handle->readbuf, 0, sizeof(handle->readbuf));
        }
    }
}


/* *******************************************************************************************************************
   ******************************************************************************************************************* */
int
upk_domain_socket_connect(const char *sockpath)
{
    struct sockaddr_un      sa = { 0 };
    int                     sa_len = sizeof(sa), fd = -2;

    strncpy(sa.sun_path, sockpath, sizeof(sa.sun_path) - 1);
    sa.sun_family = AF_UNIX;

    fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(fd >= 0) {
        if(connect(fd, (struct sockaddr *) &sa, sa_len) != 0) {
            close(fd);
            fd = -2;
        }
    }

    return fd;
}
