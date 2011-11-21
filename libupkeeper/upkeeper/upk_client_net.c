
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


#include "upk_include.h"

/*******************************************************************************************************************
 *******************************************************************************************************************/
upk_payload_t          *
upk_clnet_serial_request(upk_conn_handle_meta_t * ctrl, upk_packet_t * pkt)
{
    upk_payload_t          *pl = calloc(1, sizeof(*pl));
    struct timeval          timeout = { 30, 0 };

    UPKLIST_HEAD(ctrl);
    upk_queue_packet(ctrl->head, pkt, NULL, NULL);
    UPKLIST_HEAD(ctrl);
    upk_net_block_until_msg(ctrl, 0.1, &timeout);

    memcpy(pl, &ctrl->head->last_pkt_data, sizeof(*pl));

    UPKLIST_HEAD(ctrl);
    upk_net_flush_closed_sockets(ctrl);

    return pl;
}

/*******************************************************************************************************************
 *******************************************************************************************************************/
void
upk_clnet_req_preamble(upk_conn_handle_meta_t * ctrl)
{
    upk_packet_t           *preamble = NULL;
    upk_payload_t          *pl = NULL;

    preamble = upk_create_req_preamble(ctrl->head, upk_clientid());
    pl = upk_clnet_serial_request(ctrl, preamble);

    switch (pl->type) {
    case UPK_REPL_PREAMBLE:
        ctrl->head->version_id = ctrl->head->last_pkt_data.payload.repl_preamble.best_version;
        upk_notice("client negotiated version %d\n", ctrl->head->version_id);
        break;
    default:
        upk_fatal("Unexpected reply during handshake\n");
    }
    upk_pkt_free(preamble);
    free(pl);
}


/*******************************************************************************************************************
 *******************************************************************************************************************/
upk_conn_handle_meta_t *
upk_clnet_ctrl_connect(void)
{
    upk_conn_handle_meta_t *ctrl = NULL;
    int                     fd;

    ctrl = upk_net_conn_handle_init(NULL, NULL);
    fd = upk_domain_socket_connect(upk_runtime_configuration.controller_socket);

    if(fd >= 0) {
        UPKLIST_HEAD(ctrl);
        upk_net_add_socket_handle(ctrl, fd, NULL);
        upk_clnet_req_preamble(ctrl);
    }
    UPKLIST_HEAD(ctrl);

    return ctrl;
}

/*******************************************************************************************************************
 *******************************************************************************************************************/
void
upk_clnet_req_disconnect(upk_conn_handle_meta_t * ctrl)
{
    upk_packet_t           *req = NULL;
    upk_payload_t          *pl = NULL;

    if(ctrl->head) {
        req = upk_create_req_disconnect(ctrl->head);
        pl = upk_clnet_serial_request(ctrl, req);
        upk_pkt_free(req);
        switch (pl->type) {
        case UPK_REQ_DISCONNECT:
            upk_debug1("received a disconnect\n");
        default:
            UPKLIST_HEAD(ctrl);
            upk_disconnect_handle(ctrl);
        }
        free(pl);
    }
    upk_net_flush_closed_sockets(ctrl);
}


/*******************************************************************************************************************
 *******************************************************************************************************************/
void
upk_clnet_ctrl_disconnect(upk_conn_handle_meta_t * ctrl)
{
    upk_clnet_req_disconnect(ctrl);
    UPKLIST_FREE(ctrl);
}

/*******************************************************************************************************************
 *******************************************************************************************************************/
bool
upk_clnet_req_action(upk_conn_handle_meta_t * ctrl, char *svc_id, char *action)
{
    upk_packet_t           *req = NULL;
    upk_payload_t          *pl = NULL;
    bool                    success = false;

    req = upk_create_req_action(ctrl->head, svc_id, action);
    pl = upk_clnet_serial_request(ctrl, req);

    switch (pl->type) {
    case UPK_REPL_RESULT:
        upk_notice("%s\n", pl->payload.repl_result.msg);
        success = pl->payload.repl_result.successful;
        break;
    case UPK_REPL_ERROR:
        if(pl->payload.repl_error.errlevel > upk_diag_verbosity)
            upk_warn("Controller encountered the following error: %s\n", pl->payload.repl_error.msg);
    default:
        upk_error("unexpected reply\n");
    }
    upk_pkt_free(req);
    free(pl);
    return success;
}

/*******************************************************************************************************************
 *******************************************************************************************************************/
bool
upk_clnet_req_signal(upk_conn_handle_meta_t * ctrl, char *svc_id, upk_signal_t signal, bool should_signal_sid,
                     bool should_signal_pgrp)
{
    upk_packet_t           *req = NULL;
    upk_payload_t          *pl = NULL;
    bool                    success = false;

    req = upk_create_req_signal(ctrl->head, svc_id, signal, should_signal_sid, should_signal_pgrp);
    pl = upk_clnet_serial_request(ctrl, req);

    switch (pl->type) {
    case UPK_REPL_RESULT:
        upk_notice("%s\n", pl->payload.repl_result.msg);
        success = pl->payload.repl_result.successful;
        break;
    case UPK_REPL_ERROR:
        if(pl->payload.repl_error.errlevel > upk_diag_verbosity)
            upk_warn("Controller encountered the following error: %s\n", pl->payload.repl_error.msg);
    default:
        upk_error("unexpected reply\n");
    }
    upk_pkt_free(req);
    free(pl);
    return success;
}

/*******************************************************************************************************************
 *******************************************************************************************************************/
upk_svcinfo_t          *
upk_clnet_req_status(upk_conn_handle_meta_t * ctrl, char *svc_id, uint32_t restart_window_seconds)
{
    upk_packet_t           *req = NULL;
    upk_payload_t          *pl = NULL;
    upk_svcinfo_t          *svcinfo = calloc(1, sizeof(*svcinfo));

    req = upk_create_req_status(ctrl->head, svc_id, restart_window_seconds);
    pl = upk_clnet_serial_request(ctrl, req);

    switch (pl->type) {
    case UPK_REPL_SVCINFO:
        *svcinfo = pl->payload.repl_svcinfo.svcinfo;
        break;
    case UPK_REPL_ERROR:
        if(pl->payload.repl_error.errlevel > upk_diag_verbosity)
            upk_warn("Controller encountered the following error: %s\n", pl->payload.repl_error.msg);
    default:
        upk_error("unexpected reply\n");
    }
    upk_pkt_free(req);
    free(pl);

    return svcinfo;
}
