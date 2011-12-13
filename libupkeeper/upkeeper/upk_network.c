
/** *************************************************************************
 * Copyright (c) 2011 Yahoo! Inc. All rights reserved. Licensed under the
 * Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License
 * at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable
 * law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 * See accompanying LICENSE file. 
 ****************************************************************************/

/**
  @file

  The networking code shared by controller and clients

  @addtogroup upk_network
  @{
  */

#include "upk_include.h"
static void             upk_call_received_packet_callbacks(upk_conn_handle_meta_t * handles, upk_packet_t * pkt);

/** *****************************************************************************************************************
  @brief initialize file-descriptor sets

  @param[in] handles                the list of client handles
  @param[out] readfds               the read fdset
  @param[out] writefds              the write fdset
  @param[out] exceptfds             the exception fdset

  @return highest fd in any set. (for select)

FIXME: this does not presently handle fdset partitioning, which is necessary for poll-like behavior; this should be fixed
if we want to support more than a small number of concurent clients (<= 1024 on many platforms; limited to FD_SETSIZE) 
 ********************************************************************************************************************/
static inline int
upk_build_fd_sets(upk_conn_handle_meta_t * handles, fd_set * readfds, fd_set * writefds, fd_set * exceptfds)
{
    uint32_t                max_fds = 0;

    FD_ZERO(readfds);
    FD_ZERO(writefds);
    FD_ZERO(exceptfds);

    UPKLIST_FOREACH(handles) {
        if(handles->thisp->fd > 0 && fcntl(handles->thisp->fd, F_GETFL) >= 0) {
            FD_SET(handles->thisp->fd, readfds);                                         /* readfds */
            FD_SET(handles->thisp->fd, writefds);                                        /* writefds */
            FD_SET(handles->thisp->fd, exceptfds);                                       /* exceptfds */
            max_fds = (handles->thisp->fd > max_fds) ? handles->thisp->fd : max_fds;     /* local maxima */
        }
    }
    return max_fds;
}

/** *****************************************************************************************************************
  @brief userdata cleanup callback

  @param ptr                        the userdata object to free
 ********************************************************************************************************************/
static void
upk_partitioned_userdata_free(void *ptr)
{
    upklist_userdata_state_partition_t *data = ptr;

    if(!data)
        return;

    if(data->global_userdata) {
        if(data->userdata_free_func)                                                     /* clean using free_func */
            data->userdata_free_func(data->global_userdata);
        else
            free(data->global_userdata);                                                 /* otherwise, clean with free() */
    }

    UPKLIST_FREE(data->global_state->callback_stack);                                    /* cleanup global callback_stack (initial stack) */

    free(data->global_state);
    free(ptr);
}


/** *****************************************************************************************************************
  @brief initialize connection handles 

  @param[in] userdata               pointer to any data you want to have available to connection handles
  @param[in] userdata_free_func     the function to free userdata

  @return an initialized connection handles llist metanode
 ********************************************************************************************************************/
upk_conn_handle_meta_t *
upk_net_conn_handles_init(void *userdata, void (*userdata_free_func) (void *ptr))
{
    upk_conn_handle_meta_t *handles = calloc(1, sizeof(*handles));
    upklist_userdata_state_partition_t *data = calloc(1, sizeof(*data));                 /* struct to split up userdata into two pieces,
                                                                                            one piece for the initial callback_stack and
                                                                                            writeq_pending; the other piece for nested
                                                                                            generic userdata */

    data->global_state = calloc(1, sizeof(*data->global_state));
    data->global_state->callback_stack = calloc(1, sizeof(*data->global_state->callback_stack));

    data->global_userdata = userdata;
    data->userdata_free_func = userdata_free_func;

    handles->userdata = data;
    handles->userdata_free_func = upk_partitioned_userdata_free;                         /* our own free-function which uses the
                                                                                            free-function provided to cleanup the nested
                                                                                            userdata, and then cleans up our nested stuff */
    return handles;
}

/** *****************************************************************************************************************
  @brief extract the nested global_state component from the generic (void *) userdata element

  @param handles                    connection handle llist

  @return global state.
 ********************************************************************************************************************/
upk_net_state_t *
upk_net_get_global_state(upk_conn_handle_meta_t * handles)
{
    upklist_userdata_state_partition_t *data = handles->userdata;

    return data->global_state;
}

/** *****************************************************************************************************************
  @brief extract nested the global_userdata component from the generic (void *) userdata element

  @param[in] handles                connection handles llist.

  @return the global_userdata element from handles->userdata (handles->userdata is split into two pieces)
 ********************************************************************************************************************/
inline void            *
upk_net_get_global_userdata(upk_conn_handle_meta_t * handles)
{
    upklist_userdata_state_partition_t *data = handles->userdata;

    return data->global_userdata;
}


/** *****************************************************************************************************************
  @brief add a new socket handle to the handles list

  @param[in,out] handles            connection handles llist, will be modified by appending a connection handle.
  @param[in] fd                     fd number of socket for new connection.

  @return true if socket is valid and connection handle appended, false otherwise
 ********************************************************************************************************************/
bool
upk_net_add_socket_handle(upk_conn_handle_meta_t * handles, int fd)
{
    int                     sockopts = 0;
    upk_net_state_t        *global_state = upk_net_get_global_state(handles);

    if(fd > 0 && (sockopts = fcntl(fd, F_GETFL))) {
        fcntl(fd, F_SETFL, sockopts | O_NONBLOCK);
        fcntl(fd, F_SETFD, FD_CLOEXEC);

        UPKLIST_APPEND(handles);
        handles->thisp->userdata = upk_net_get_global_userdata(handles);
        handles->thisp->state = calloc(1, sizeof(*handles->thisp->state));
        handles->thisp->state->callback_stack = calloc(1, sizeof(*handles->thisp->state->callback_stack));

        /* deep copy initialize this items state with the global state (which serves as initial state) */
//        UPKLIST_APPEND(handles->thisp->state->callback_stack);
        if(global_state->callback_stack->head)
            upk_callback_stack_push(handles->thisp, global_state->callback_stack->head);

        /*
        if(global_state->callback_stack->head) {
            memcpy(handles->thisp->state->callback_stack->thisp, global_state->callback_stack->head,
                   sizeof(*global_state->callback_stack->head) - sizeof(global_state->callback_stack->head->next));
        }
        */


        handles->thisp->fd = fd;
        return true;
    }
    return false;
}

/** *****************************************************************************************************************
  @brief iterate through the list of handles, and prune any that are no longer valid. 
  
  free callback stacks, userdata, and state. Unlink node from list.

  @param[in,out] handles            connection handles llist, will be modified by removing closed connection handles.
 ********************************************************************************************************************/
void
upk_net_flush_closed_sockets(upk_conn_handle_meta_t * handles)
{
    UPKLIST_FOREACH(handles) {
        if(handles->thisp->fd < 0) {                                                     /* closed fd; we set them to -2 if closed, other
                                                                                            things set them to -1 */
            if(handles->thisp->userdata) {
                if(handles->thisp->userdata_free_func)                                   /* cleanup using free_func */
                    handles->thisp->userdata_free_func(handles->thisp->userdata);
                else
                    free(handles->thisp->userdata);                                      /* otherwise just free it */
            }
            if(handles->thisp->state) {                                                  /* cleanup state; including callback_stack */
                UPKLIST_FREE(handles->thisp->state->callback_stack);
                free(handles->thisp->state);
            }
            UPKLIST_UNLINK(handles);
        }
    }
}

/** *****************************************************************************************************************
  @brief The event dispatcher, called via an event-loop. 

  This is where events in either the state-stack are calledis invoked 

  @param[in] handles                the list of handles
  @param[in] sel_ival               the wait time for select to wait
 ********************************************************************************************************************/
void
upk_net_event_dispatcher(upk_conn_handle_meta_t * handles, double sel_ival)
{
    fd_set                  readfds, writefds, exceptfds, *writefds_p = NULL;
    int                     nfds, ready;
    sigset_t                origmask, sigmask;
    upk_net_state_t        *global_state = upk_net_get_global_state(handles);
    upk_net_cb_stk_meta_t  *callbacks = NULL;
    struct timeval          timeout = upk_double_to_timeval(sel_ival);

    if(handles && handles->thisp && handles->thisp->state)
        callbacks = handles->thisp->state->callback_stack;

    if(callbacks && callbacks->head && callbacks->head->net_dispatch_pre)                /* run the net_dispatch_pre callback, run before
                                                                                            dispatch */
        callbacks->head->net_dispatch_pre(handles, NULL);

    nfds = upk_build_fd_sets(handles, &readfds, &writefds, &exceptfds);
    writefds_p = (global_state->pending_writeq > 0) ? &writefds : NULL;                  /* If there are writes, use the constructed fdset
                                                                                            above; otherwise select on NULL for writes, to
                                                                                            avoid spinning */

    sigfillset(&sigmask);
    sigprocmask(SIG_SETMASK, &sigmask, &origmask);
    ready = select(nfds + 1, &readfds, writefds_p, NULL, &timeout);
    sigprocmask(SIG_SETMASK, &origmask, NULL);

    if(ready > 0) {
        UPKLIST_FOREACH(handles) {
            if(callbacks && callbacks->head && callbacks->head->net_dispatch_foreach)    /* call the foreach callback; this callback gets
                                                                                            invoked for every single message blindly */
                callbacks->head->net_dispatch_foreach(handles, NULL);
            if(handles->thisp->fd >= 0 && fcntl(handles->thisp->fd, F_GETFL)) {
                if(FD_ISSET(handles->thisp->fd, &readfds))                               /* upk_read_packets is responsible for calling
                                                                                            read-callbacks */
                    upk_read_packets(handles);
                if(writefds_p && FD_ISSET(handles->thisp->fd, &writefds))                /* upk_write_packets is responsible for calling
                                                                                            write-callbacks */
                    upk_write_packets(handles);
            } else {
                upk_disconnect_handle(handles);
                continue;
            }
        }
    }
    if(callbacks && callbacks->head && callbacks->head->net_dispatch_post)               /* call net_dispatch_post, if necessary */
        callbacks->head->net_dispatch_post(handles, NULL);
}


/** *****************************************************************************************************************
  @brief block until a complete message is received, up to timeout.

  Make the underlying asynchronous network communication model behave like a synchronous one by blocking.

  @param[in] handles                The llist of connection handles.
  @param[in] sel_ival               The select timeout value.
  @param[in] timeout                Global timeout to block for; distinct from the sel_ival in that if a partial
                                    packet is received, select would trigger, but the global timeout would still play.

  @return message type or error. (msgtype >= 0 if valid, < 0 if invalid; if < -1, then it represents the fd value,
  which may be useful in debugging)
 ********************************************************************************************************************/
int
upk_net_block_until_msg(upk_conn_handle_meta_t * handles, double sel_ival, struct timeval *timeout)
{
    struct timeval          start_time = { 0 };
    struct timeval          cur_time = { 0 };
    upk_conn_handle_t      *handle = handles->thisp;


    gettimeofday(&start_time, NULL);
    memset(&handle->last_pkt_data, 0, sizeof(handle->last_pkt_data));

    while(handle->fd >= 0) {
        upk_net_event_dispatcher(handles, sel_ival);                                     /* event dispatcher select()'s using sel_ival as
                                                                                            timeout */
        if(handle->last_pkt_data.type)                                                   /* can only be true if we received a msg */
            return handle->last_pkt_data.type;

        gettimeofday(&cur_time, NULL);                                                   /* handle our timeout for the block */
        if(timeout && (cur_time.tv_sec >= start_time.tv_sec + timeout->tv_sec)) {
            if(timeout && (cur_time.tv_usec >= start_time.tv_usec + timeout->tv_usec))
                return -1;
        }
    }
    return handle->fd;                                                                   /* can only be < 0 here */
}


/** *****************************************************************************************************************
  @brief call callback for received packet based on type.

  Will call the appropriate msg_handler for the packet type if availble.

  @param[in] handles                connection handle llist
  @param[in] pkt                    the packet itself
 *********************************************************************************************************************/
static void
upk_call_received_packet_callbacks(upk_conn_handle_meta_t * handles, upk_packet_t * pkt)
{
    upk_payload_t          *data = NULL;
    upk_conn_handle_t      *handle = NULL;
    upk_net_cb_stk_meta_t  *callbacks = NULL;

    if(!handles || (handles && !handles->thisp))
        return;

    if(handles->thisp->state)
        callbacks = handles->thisp->state->callback_stack;

    handle = handles->thisp;
    data = &handle->last_pkt_data;
    memset(data, 0, sizeof(*data));

    data->type = upk_get_msgtype(pkt);
    memcpy(&data->payload, pkt->payload, upk_get_msgsize(data->type));

    if(callbacks && callbacks->head && callbacks->head->msg_handlers[UPK_MSGTYPE_IDX(data->type)])
        callbacks->head->msg_handlers[UPK_MSGTYPE_IDX(data->type)] (handles, data);
}


/** *****************************************************************************************************************
  @brief disconnect the ->thisp handle in handles.

  @param[in] handles                connection handles llist.
 *********************************************************************************************************************/
void
upk_disconnect_handle(upk_conn_handle_meta_t * handles)
{
    if(handles && handles->thisp) {
        shutdown(handles->thisp->fd, SHUT_RDWR);
        close(handles->thisp->fd);
        handles->thisp->fd = -2;
    }
}

/** *****************************************************************************************************************
  @brief wrapper around upk-disconnect_handle for use as a callback.
 *********************************************************************************************************************/
void
upk_net_shutdown_callback(upk_conn_handle_meta_t * handles, upk_payload_t * msg)
{
    upk_disconnect_handle(handles);
}

/** *****************************************************************************************************************
  @brief enqueue a packet for sending

  This takes a packet and places it on the writeq for handle.

  @param[in] handles                connection handles llist
  @param[in] handle                 the handle to update the writeq for.
  @param[in] pkt                    the packet to enqueue
  @param[in] after_write_callback   a callback to call after successfully writing the packet, usually to setup the
                                    receive callback stack.

  pkt will be copied, so that the copy may be freed after it's been sent. You must free the pkt you pass yourself.
 *********************************************************************************************************************/
void
upk_queue_packet(upk_conn_handle_meta_t * handles, upk_conn_handle_t * handle, upk_packet_t * pkt, upk_net_callback_t after_write_callback)
{
    upk_pkt_buf_t          *bufp = NULL;
    upk_netmsg_queue_t     *msgp = NULL;
    upk_net_state_t        *global_state = upk_net_get_global_state(handles);

    UPKLIST_APPEND((&handle->writeq));
    global_state->pending_writeq++;                                                      /* increment pending */
    msgp = handle->writeq.thisp;                                                         /* shorthand for the code that follows */

    msgp->after_write_callback = after_write_callback;
    msgp->msg_len = UPK_PACKET_HEADER_LEN + pkt->payload_len + UPK_PACKET_FOOTER_LEN;

    bufp = upk_serialize_packet(pkt);
    memcpy(msgp->msg, bufp, msgp->msg_len);

    free(bufp);
}


/** *****************************************************************************************************************
  @brief write the packets in the writeq for handles->thisp, and call any write callbacks

  @param[in] handles                connection handles llist; uses thisp
 *********************************************************************************************************************/
void
upk_write_packets(upk_conn_handle_meta_t * handles)
{
    size_t                  n_write = 0;
    upk_netmsg_queue_t     *msgp = NULL;
    upk_conn_handle_t      *handle = handles->thisp;
    upk_net_state_t        *global_state = upk_net_get_global_state(handles);

    if(handle->fd < 0)
        return;

    if(handle->writeq.count < 1 || !handle->writeq.thisp)
        return;

    msgp = handle->writeq.thisp;

    if((msgp->msg_len - msgp->n_bytes_written) > 0) {
        n_write = write(handle->fd, msgp->msg + msgp->n_bytes_written, msgp->msg_len - msgp->n_bytes_written);
        if(n_write < 1 && !(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
            upk_disconnect_handle(handles);                                              /* disconnect on unrecoverable errors */
            return;
        }
        if(n_write < 0)
            n_write = 0;

        msgp->n_bytes_written += n_write;

        if(msgp->n_bytes_written == msgp->msg_len) {                                     /* done writing */
            if(msgp->after_write_callback)                                               /* call the after_write_callback */
                msgp->after_write_callback(handles, NULL);
            UPKLIST_UNLINK((&handle->writeq));                                           /* dequeue the message */
            global_state->pending_writeq--;                                              /* decrement pending */
        }
    }
}

/* FIXME: refactor to smaller (static inline) functions, this is too much going on in one place */
/** ******************************************************************************************************************
  @brief read any packets from the handle handles->thisp, and call read-callbacks.

  @param[in] handles connection handles llist.
  ********************************************************************************************************************/
void
upk_read_packets(upk_conn_handle_meta_t * handles)
{
    size_t                  n_read = 0;
    upk_conn_handle_t      *handle = NULL;
    uint32_t                payload_len, version_id;
    upk_packet_t           *pkt;

    if(!handles || (handles && !handles->thisp))                                         /* return if there is no thisp handle to examine */
        return;

    handle = handles->thisp;

    if(handle->fd < 0)                                                                   /* return if the handle is closed already */
        return;


    /* read the 16 header bytes, non-blocking to prevent a rogue handle from stopping up the works, so we have to nibble */
    if(handle->n_remaining_read == 0 && handle->n_hdr_bytes_read < UPK_PACKET_HEADER_LEN) {
        errno = 0;
        n_read = read(handle->fd, handle->readbuf, UPK_PACKET_HEADER_LEN - handle->n_hdr_bytes_read);
        if(n_read < 1 && !(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
            upk_disconnect_handle(handles);
            return;
        }
        handle->n_hdr_bytes_read += (n_read < 0) ? 0 : n_read;                           /* if read encountered an error, don't subtract */
    }


    /* verify header is something we can handle */
    if(handle->n_remaining_read == 0 && handle->n_hdr_bytes_read == UPK_PACKET_HEADER_LEN) {
        payload_len = ntohl(*((uint32_t *) handle->readbuf));
        version_id = ntohl(*((uint32_t *) handle->readbuf + 1));
        handle->n_hdr_bytes_read = 0;

        if(!(version_id >= UPK_MIN_SUPPORTED_PROTO && version_id <= UPK_MAX_SUPPORTED_PROTO)) {
            upk_error("received invalid packet: invalid version_id: %d\n", version_id);
            upk_disconnect_handle(handles);                                              /* disconnect on bogus packets */
            return;
        }
        if(payload_len > UPK_MAX_PACKET_SIZE) {
            upk_error("received invalid packet: invalid payload_len: %d\n", payload_len);
            upk_disconnect_handle(handles);                                              /* disconnect on bogus packets */
            return;
        }
        handle->n_remaining_read = payload_len + UPK_PACKET_FOOTER_LEN;
        upk_debug1("reading packet of length %d\n", handle->n_remaining_read);
    }


    /* after reading and validating header, read the prescribed number of bytes, also nibbling */
    if(handle->n_remaining_read > 0) {
        n_read = read(handle->fd, handle->readbuf + UPK_PACKET_HEADER_LEN, handle->n_remaining_read);
        if(n_read < 1 && !(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
            upk_disconnect_handle(handles);                                              /* disconnect on unrecoverable errors */
            return;
        }

        handle->n_remaining_read -= (n_read < 0) ? 0 : n_read;                           /* if read encountered an error, don't subtract */
        upk_debug1("read %d bytes of packet (%d remaining)\n", n_read, handle->n_remaining_read);

        if(handle->n_remaining_read == 0) {                                              /* entire packet receieved */
            pkt = upk_deserialize_packet(handle->readbuf);
            if(!pkt) {                                                                   /* upk_deserialize_packet returns NULL if it
                                                                                            encounters an error */
                upk_error("Invalid packet\n");
            } else {
                upk_call_received_packet_callbacks(handles, pkt);
                upk_pkt_free(pkt);
            }
            memset(handle->readbuf, 0, sizeof(handle->readbuf));                         /* zero out the readbuf for next round */
        }
    }
}


/** *****************************************************************************************************************
  @brief connect to a domain socket.

  @param[in] sockpath               Path to the socket in question.

  @return the fd of the socket, or -2 on connect error (-1 if socket() error, errno may be checked)
 ********************************************************************************************************************/
int
upk_domain_socket_connect(const char *sockpath)
{
    struct sockaddr_un      sa = { 0 };
    int                     sa_len = sizeof(sa), fd = -2;

    strncpy(sa.sun_path, sockpath, sizeof(sa.sun_path) - 1);
    sa.sun_family = AF_UNIX;

    errno = 0;
    fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(fd >= 0) {
        errno = 0;
        if(connect(fd, (struct sockaddr *) &sa, sa_len) != 0) {
            close(fd);
            fd = -2;
        }
    }

    return fd;
}


/** *****************************************************************************************************************
  @brief push a callback-stack state element onto the callback stack for a handle.

  @param[in] handle                 the handle to push the state onto.
  @param[in] state                  The state element to push.
 ********************************************************************************************************************/
void
upk_callback_stack_push(upk_conn_handle_t * handle, upk_net_cb_stk_t * state)
{
    UPKLIST_PREPEND(handle->state->callback_stack);
    *handle->state->callback_stack->thisp = *state;
}

/** *****************************************************************************************************************
  @brief pop the top element off the callback-stack for a handle.

  @param[in] handle                 The handle to pop the state from.

  Note that the element popped will be freed, and should not be referenced after popping. This does not return
  the popped element; it just discards it.
 ********************************************************************************************************************/
void
upk_callback_stack_pop(upk_conn_handle_t * handle)
{
    UPKLIST_HEAD(handle->state->callback_stack);
    UPKLIST_UNLINK(handle->state->callback_stack);
}


/**
  @}
  */
