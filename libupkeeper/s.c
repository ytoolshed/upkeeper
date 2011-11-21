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

int crap(upk_msgtype_t type)
{
    static size_t msgsizes[] = {
           sizeof(upk_req_preamble_t),
           sizeof(upk_req_seq_start_t),
           sizeof(upk_req_seq_end_t),
           sizeof(upk_req_action_t),
           sizeof(upk_req_signal_t),
           sizeof(upk_req_list_t),
           sizeof(upk_req_status_t),
           sizeof(upk_req_subscribe_t),
           sizeof(upk_req_unsubscribe_t),
           sizeof(upk_req_disconnect_t),

           sizeof(upk_repl_preamble_t),
           sizeof(upk_repl_seq_start_t),
           sizeof(upk_repl_seq_end_t),
           sizeof(upk_repl_result_t),
           sizeof(upk_repl_listing_t),
           sizeof(upk_repl_svcinfo_t),
           sizeof(upk_repl_ack_t),
           sizeof(upk_repl_error_t),

           sizeof(upk_pub_publication_t),
           sizeof(upk_pub_cancelation_t),
    };
    char msgnames[][32] = {
           "upk_req_preamble_t",
           "upk_req_seq_start_t",
           "upk_req_seq_end_t",
           "upk_req_action_t",
           "upk_req_signal_t",
           "upk_req_list_t",
           "upk_req_status_t",
           "upk_req_subscribe_t",
           "upk_req_unsubscribe_t",
           "upk_req_disconnect_t",

           "upk_repl_preamble_t",
           "upk_repl_seq_start_t",
           "upk_repl_seq_end_t",
           "upk_repl_result_t",
           "upk_repl_listing_t",
           "upk_repl_svcinfo_t",
           "upk_repl_ack_t",
           "upk_repl_error_t",

           "upk_pub_publication_t",
           "upk_pub_cancelation_t",
    };

    /* for every supported protocol there will be a (REQ|REPL|PUB)_V<N>_LIMIT entry in the enum;
       hence, substracting n_supported_protocols from FOO_LIMIT should yield the highest actual enum value per range */

#define n_protos (1 + UPK_MAX_SUPPORTED_PROTO - UPK_MIN_SUPPORTED_PROTO)
#define n_per(A) (UPK_##A##_LIMIT - UPK_##A##_ORIGIN - n_protos)
#define t_scs(A,B) (type < UPK_##A##_LIMIT) ? (type - UPK_##A##_ORIGIN) + B 
#define t_idx t_scs(REQ, 0) : t_scs(REPL, n_per(REQ)) : t_scs(PUB, n_per(REQ) + n_per(REPL)) : -1

    printf("%s\n", msgnames[t_idx]);
    return msgsizes[t_idx];
}

int
main(int argc, char **argv, char **envp)
{
    upk_conn_handle_meta_t *clients = upk_net_conn_handle_init(NULL, NULL);
    upk_conn_handle_meta_t *ctrl = upk_net_conn_handle_init(NULL, NULL);
    upk_packet_t           *pkt = NULL;
    int pair[2];
/*    
    upk_diag_verbosity = UPK_DIAGLVL_DEBUG1;

#define tcrap(A) printf("%s ==> ",stringify(A)); crap(A) 
    tcrap(UPK_REQ_ACTION);
    tcrap(UPK_REQ_DISCONNECT);
    tcrap(UPK_REPL_RESULT);
    tcrap(UPK_PUB_PUBLICATION);
    return 0;
    */

    socketpair(AF_UNIX, SOCK_STREAM, 0, pair);
    if(pair[0] < 0 || pair[1] < 0)
        upk_fatal("unable to open socketpair\n");

    upk_net_add_socket_handle(ctrl, pair[0], NULL);
    upk_net_add_socket_handle(clients, pair[1], NULL);

    if(argc > 1 && (strcmp(argv[1], "action") == 0))
        pkt = upk_create_req_action(ctrl->head, "foobar", "start");
    else
        pkt = upk_create_req_preamble(ctrl->head, upk_clientid());

    upk_queue_packet(ctrl->head, pkt, NULL, NULL);
    //upk_pkt_free(pkt);

    while(!clients->head->last_pkt_data.type) { 
        upk_net_event_dispatcher(ctrl, 0.1);
        upk_net_event_dispatcher(clients, 0.1);
    }

    write(fileno(stdout), clients->head->last_pkt_data.payload.req_preamble.client_name, strlen(clients->head->last_pkt_data.payload.req_preamble.client_name));
    return 0;
}
