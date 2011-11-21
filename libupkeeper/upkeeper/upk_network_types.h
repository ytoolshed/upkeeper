
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

#ifndef _UPK_NETWORK_TYPES_H
#define _UPK_NETWORK_TYPES_H

/**
  @file
  */

/**
  @addtogroup upk_network
  @{
  */


typedef struct _upk_conn_handle upk_conn_handle_t;
typedef                 UPKLIST_METANODE(upk_conn_handle_t, upk_conn_handle_meta_p), upk_conn_handle_meta_t;

/**
  @brief typedef for callback functions.

  @param[in] handles - the handle list, where thisp is the handle that generated the event for the callback.
  @param[in,out] data - used to pass data you specify to the callbacks.
  @param[in] msg - the deserialized message.
  */
typedef void            (*upk_net_callback_t) (upk_conn_handle_meta_t * handles, upk_payload_t * msg);


/**
  @brief this is a tailq for network message queuing 
  */
typedef struct _upk_netmsg_queue upk_netmsg_queue_t;

/**
  @brief this is a tailq for network message queuing 
  */
struct _upk_netmsg_queue {
    upk_pkt_buf_t           msg[UPK_MAX_PACKET_SIZE];      /*!< serialized packet (a message) */
    size_t                  msg_len;                       /*!< length of msg to write */
    size_t                  n_bytes_written;               /*!< remaining bytes in message to write ; used to advance ptr */
    upk_net_callback_t      after_write_callback;          /*!< call this after writing this msg; may be NULL */
    upk_net_callback_t      set_after_read_callback;       /*!< will set the handle's "after_read_callback" to this function after this message is written; may be NULL */
    upk_netmsg_queue_t     *next;                          /*!< next */
};

typedef                 UPKLIST_METANODE(upk_netmsg_queue_t, upk_netmsg_queue_meta_p), upk_netmsg_queue_meta_t;

/**
  @brief a callback-stack for use in nesting/branching expectations in a conversation
  */
typedef struct _upk_net_cb_stk upk_net_cb_stk_t;

#define TOTAL_N_MSG_TYPES (UPK_N_REQ_TYPES + UPK_N_REPL_TYPES + UPK_N_PUB_TYPES)

/**
  @brief a callback-stack for use in nesting/branching expectations in a conversation
  */
struct _upk_net_cb_stk {
    upk_net_callback_t      msg_handlers[TOTAL_N_MSG_TYPES];    /*!< the array of callback handlers for each type of message, set the
                                                                   appropriate bucket with the function that handles the type of messages
                                                                   you expect next. Use the macro UPK_MSGTYPE_IDX(msgtype) to find the
                                                                   appropriate bucket */
    upk_net_callback_t      net_dispatch_pre;              /*!< before dispatching the event to the handler in msg_handlers */
    upk_net_callback_t      net_dispatch_foreach;          /*!< run this for every message */
    upk_net_callback_t      net_dispatch_post;             /*!< run this after dispatching to a handler */
    upk_net_cb_stk_t       *next;                          /*!< next */
};
/**
  @brief metanode for the callback stack
  */
typedef                 UPKLIST_METANODE(upk_net_cb_stk_t, upk_net_cb_stk_meta_p), upk_net_cb_stk_meta_t;

/**
  @brief global state data for this client/connection
  */
typedef struct _upk_net_gstate upk_net_gstate_t;
struct _upk_net_gstate {
    size_t                  pending_writeq;                /*!< how many messages are waiting to be sent, used to determine if select on
                                                              write is necessary */
    upk_net_cb_stk_meta_t  *callback_stack;                /*!< the callback stack, who's current head is at this point in the
                                                              conversation */
};

/**
  @brief a struct to use as 'userdata' to be passed around, which partitions the 'userdata' segment into global state and then any other userdata you might care about.
  */
typedef struct _upklist_userdata_state_partition upklist_userdata_state_partition_t;
/**
  @brief a struct to use as 'userdata' to be passed around, which partitions the 'userdata' segment into global state and then any other userdata you might care about.
  */
struct _upklist_userdata_state_partition {
    void                   *userdata;                      /*!< generic place to stuff data you might need */
    upk_net_gstate_t       *gstate;                        /*!< global state data for this handle */
    void                    (*userdata_free_func) (void *); /*!< function to free your userdata */
};

struct _upk_conn_handle {
    uint32_t                version_id;                    /*!< proto version for connection */
    int                     fd;                            /*!< file descriptor */
    char                    cl_name[UPK_MAX_STRING_LEN];   /*!< name of associated client */
    upk_pkt_buf_t           readbuf[UPK_MAX_PACKET_SIZE];  /*!< readbuffer used in async read */
    uint8_t                 n_hdr_bytes_read;              /*!< remaining header bytes for async read */
    size_t                  n_remaining_read;              /*!< reamining bytes after header for async read */
    upk_net_callback_t      after_read_callback;           /*!< after an entire packet read, call this callback */
    upk_netmsg_queue_meta_t writeq;                        /*!< write queue linked list */
    upk_payload_t           last_pkt_data;                 /*!< Last received packet data */
    upk_net_gstate_t       *gstate;                        /*!< instance state data, maintained in list metanode */
    void                   *userdata;                      /*!< any user-data you might need too pass around */
    upk_conn_handle_t      *next;                          /*!< next */
};

/**
  @}
  */

#endif
