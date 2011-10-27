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

#ifndef _UPK_CONTROLLER_H
#define _UPK_CONTROLLER_H

#include <upkeeper.h>
#include <stdio.h>
#include <ctrl_protocol.h>

#define UPK_CTRL_MAX_CLIENTS FD_SETSIZE

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
