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

int
main(int argc, char **argv, char **envp)
{
    char                   *svc_id;
    upk_protocol_handle_t   handle = { 0 };
    upk_packet_t           *req = NULL;
    upk_packet_t           *reply = NULL;
    size_t reply_payload_len = 0;
    struct sockaddr_un      sa = { 0 };
    int                     sa_len = sizeof(sa), fd = -2;
    upk_pkt_buf_t          *ptr;
    upk_pkt_buf_t reply_buf[UPK_MAX_PACKET_SIZE] = "";

    upk_diag_verbosity = UPK_DIAGLVL_DEBUG1;

    upk_ctrl_load_config();

    if(argc != 2)
        upk_fatal("Must provide service id\n");

    svc_id = argv[1];
    req = upk_create_req_action(&handle, svc_id, "stop");
    upk_debug1("version_id: %d\n", req->version_id);

    upk_verbose("opening socket: %s\n", upk_runtime_configuration.controller_socket);
    strncpy(sa.sun_path, upk_runtime_configuration.controller_socket, sizeof(sa.sun_path) - 1);
    sa.sun_family = AF_UNIX;

    fd = socket(PF_UNIX, SOCK_STREAM, 0);

    if(connect(fd, (struct sockaddr *) &sa, sa_len) == 0) {
        ptr = upk_serialize_packet(req);
        write(fd, ptr, (sizeof(*req) - sizeof(req->payload)) + req->payload_len);
        free(ptr);

        read(fd, reply_buf, 16);
        reply_payload_len = ntohl( *((uint32_t *) reply_buf) );

        read(fd, reply_buf+16, reply_payload_len + 4);
        reply = upk_deserialize_packet(reply_buf);

        if(reply && reply->payload) {
            upk_notice("%s\n", ((upk_repl_result_t *) reply->payload)->msg);
        }

        shutdown(fd, SHUT_RDWR);
        close(fd);
    } else {
        upk_fatal("unable to connect to controoler\n");
    }

    if(req)
        upk_pkt_free(req);
    if(reply)
        upk_pkt_free(reply);

    return 0;
}
