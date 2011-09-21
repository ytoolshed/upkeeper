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

#ifndef _UPK_CLIENT_NET_H
#define _UPK_CLIENT_NET_H

/* upkeeper/upk_client_net.c */
extern upk_payload_t *upk_clnet_serial_request(upk_conn_handle_meta_t *ctrl, upk_packet_t *pkt);
extern void upk_clnet_req_preamble(upk_conn_handle_meta_t *ctrl);
extern upk_conn_handle_meta_t *upk_clnet_ctrl_connect(void);
extern void upk_clnet_req_disconnect(upk_conn_handle_meta_t *ctrl);
extern void upk_clnet_ctrl_disconnect(upk_conn_handle_meta_t *ctrl);
extern _Bool upk_clnet_req_action(upk_conn_handle_meta_t *ctrl, char *svc_id, char *action);

#endif
