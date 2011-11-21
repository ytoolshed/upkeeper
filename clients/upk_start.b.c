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

#include <upkeeper.h>
#include <sys/un.h>
#include <stdio.h>

void
received_packet_callback(upk_conn_handle_meta_t * controller, upk_payload_t * msg)
{
    if(msg->type == UPK_REPL_RESULT) {
        upk_notice("%s\n", msg->payload.repl_result.msg);
        shutdown(controller->head->fd, SHUT_RDWR);
        close(controller->head->fd);
        controller->head->fd = -2;
    }
}


int
main(int argc, char **argv, char **envp)
{
    char                   *svc_id;
    upk_conn_handle_meta_t *controller;
    upk_packet_t           *req = NULL;
    struct sockaddr_un      sa = { 0 };
    int                     sa_len = sizeof(sa), fd = -2;

    upk_diag_verbosity = UPK_DIAGLVL_DEBUG1;

    upk_ctrl_load_config();

    if(argc != 2)
        upk_fatal("Must provide service id\n");

    svc_id = argv[1];

    controller = calloc(1, sizeof(*controller));

    upk_verbose("opening socket: %s\n", upk_runtime_configuration.controller_socket);
    strncpy(sa.sun_path, upk_runtime_configuration.controller_socket, sizeof(sa.sun_path) - 1);
    sa.sun_family = AF_UNIX;

    fd = socket(PF_UNIX, SOCK_STREAM, 0);

    if(connect(fd, (struct sockaddr *) &sa, sa_len) == 0) {
        upk_net_add_socket_handle(controller, fd, NULL);
        req = upk_create_req_action(&controller->head->proto_handle, svc_id, "start");
        upk_queue_packet(controller->head, req, NULL, received_packet_callback);
        upk_pkt_free(req);
    } else {
        upk_fatal("unable to connect to controoler\n");
    }

    while(controller->head && controller->head->fd >= 0) {
        upk_net_event_dispatcher(controller,-1.0);
    }

    return 0;
}
