
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

#ifndef _UPK_NETWORK_H
#define _UPK_NETWORK_H

#include "upk_include.h"

/**
  @file
  */

extern int              upk_domain_socket_connect(const char *sockpath);
extern void             upk_read_packets(upk_conn_handle_meta_t * handles);
extern void             upk_queue_packet(upk_conn_handle_meta_t * handles, upk_conn_handle_t * handle, upk_packet_t * pkt,
                                         upk_net_callback_t after_write_callback);
extern void             upk_write_packets(upk_conn_handle_meta_t * handles);
extern void             upk_disconnect_handle(upk_conn_handle_meta_t * handles);
extern bool             upk_net_add_socket_handle(upk_conn_handle_meta_t * handles, int fd);
extern void             upk_net_event_dispatcher(upk_conn_handle_meta_t * handles, double poll_ival);
extern void             upk_net_flush_closed_sockets(upk_conn_handle_meta_t * handles);
extern int              upk_net_block_until_msg(upk_conn_handle_meta_t * handles, double poll_ival, struct timeval *timeout);
extern upk_conn_handle_meta_t *upk_net_conn_handles_init(void *userdata, void (*userdata_free_func) (void *ptr));
extern void             upk_net_shutdown_callback(upk_conn_handle_meta_t * handles, upk_payload_t * msg);
upk_net_state_t        *upk_net_get_global_state(upk_conn_handle_meta_t * handles);
void                    upk_callback_stack_push(upk_conn_handle_t * handle, upk_net_cb_stk_t * state);
void                    upk_callback_stack_pop(upk_conn_handle_t * handle);


#endif
