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

#ifndef _UPK_CONTROLLER_H
#define _UPK_CONTROLLER_H

#include <upkeeper.h>
#include <stdio.h>
#include <ctrl_protocol.h>

#define UPK_CTRL_MAX_CLIENTS FD_SETSIZE

/*

typedef struct _client_fdlist client_fdlist_t;
struct _client_fdlist {
    int32_t                 fd;
    upk_pkt_buf_t           readbuf[UPK_MAX_PACKET_SIZE];
    uint8_t                 n_hdr_bytes_read;
    size_t                  n_remaining_read;
    upk_pkt_buf_t           writebuf[UPK_MAX_PACKET_SIZE];
    size_t                  n_remaining_write;
    client_fdlist_t        *next;
};

*/

/* typedef UPKLIST_METANODE(client_fdlist_t, client_fdlist_head_p), client_fdlist_head_t; */

/*
typedef struct _buddylist buddylist_t;
struct _buddylist {
    char                    buddy_name[UPK_MAX_STRING_LEN];
    char                    custom_actions[32][UPK_MAX_STRING_LEN];
    buddylist_t            *next;
};
*/

//extern upk_controller_config_t *upk_ctrl_config;


extern int              socket_setup(const char *sock_path);
extern void             handle_signals(void);
extern void             handle_buddies(void);

/* XXX: refactoring */
extern int32_t          ctrl_sock_setup(void);
extern void             event_loop(int32_t listen_sock);

extern int upk_ctrl_init(void);


/* ctrl_buddy.c */
extern void create_buddy(upk_svc_desc_t *buddy);
extern void remove_buddy(upk_svc_desc_t *buddy);
extern int buddy_connect(const char *sockpath);
buddy_info_t * buddy_readstate(int fd);
extern void handle_buddies(void);
extern bool kill_buddy(upk_svc_desc_t *buddy);
extern bool start_buddy_svc(upk_svc_desc_t *buddy);
extern bool stop_buddy_svc(upk_svc_desc_t *buddy);
extern bool reload_buddy_svc(upk_svc_desc_t *buddy);



#endif
