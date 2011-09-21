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

#define _stringify(A) #A
#define stringify(A) _stringify(A)

#define _create_fmt(FMT) ": got: " stringify(FMT) ", wanted: " stringify(FMT)

#define check_isequal(A, B, FMT) \
                fail_unless( (A == B), stringify(A) _create_fmt(FMT), A, B)

#define check_isequal_str(A, B, FMT) \
                fail_unless( strcmp(A, B) == 0, stringify(A) _create_fmt(FMT), A, B)

#ifdef CHECK_PREAMBLE
START_TEST(test_proto_req_preamble)
{
    TEST_SETUP(req_preamble, "clientname");

    check_isequal(t.msgtype, UPK_REQ_PREAMBLE, %d);
    check_isequal(t.min_supported_ver, UPK_MIN_SUPPORTED_PROTO, %d);
    check_isequal(t.max_supported_ver, UPK_MAX_SUPPORTED_PROTO, %d);
    check_isequal(t.client_name_len, strlen("clientname"), %d);
    check_isequal_str(t.client_name, "clientname", %s);
    check_isequal(pkt->pkttype, PKT_REQUEST, %d);
    check_isequal(pkt->payload_len, (4 + 4 + 4 + 4) + strlen("clientname"), %d);

    TEST_TEARDOWN;
}
END_TEST;

START_TEST(test_proto_repl_preamble)
{
    TEST_SETUP(repl_preamble, 4);

    check_isequal(t.msgtype, UPK_REPL_PREAMBLE, %d);
    check_isequal(t.best_version, 4, %d);

    check_isequal(pkt->pkttype, PKT_REPLY, %d);
    check_isequal(pkt->payload_len, (4 + 4), %d);

    TEST_TEARDOWN;
}
END_TEST;

#endif

START_TEST(test_proto_req_seq_start)
{
    TEST_SETUP(req_seq_start, UPK_REQ_LIST, 15);

    fail_unless(t.msgtype == UPK_REQ_SEQ_START, "msgtype");
    fail_unless(t.msg_seq_type == UPK_REQ_LIST, "msg_seq_type");
    fail_unless(t.msg_seq_count == 15, "msg_seq_count");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == (4 + 4 + 4), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_req_seq_end)
{
    TEST_SETUP(req_seq_end, true);

    fail_unless(t.msgtype == UPK_REQ_SEQ_END, "msgtype");
    fail_unless(t.commit == true, "commit");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == (4 + 1), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_req_action)
{
    TEST_SETUP(req_action, "some service", "some action");

    fail_unless(t.msgtype == UPK_REQ_ACTION, "msgtype");
    fail_unless(t.svc_id_len == strlen("some service"), "svc_id_len");
    fail_unless(strcmp(t.svc_id, "some service") == 0, "svc_id");
    fail_unless(t.action_len == strlen("some action"), "action_len");
    fail_unless(strcmp(t.action, "some action") == 0, "action");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == 4 + 4 + strlen("some service") + 4 + strlen("some action"), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_req_signal)
{
    TEST_SETUP(req_signal, "some service", UPK_SIG_KILL, true, true);


    fail_unless(t.msgtype == UPK_REQ_SIGNAL, "msgtype");
    fail_unless(t.svc_id_len == strlen("some service"), "svc_id_len");
    fail_unless(strcmp(t.svc_id, "some service") == 0, "svc_id");
    fail_unless(t.signal == UPK_SIG_KILL, "signal");
    fail_unless(t.signal_sid == true, "signal_sid");
    fail_unless(t.signal_pgrp == true, "signal_pgrp");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == 4 + 4 + strlen("some service") + 4 + 1 + 1, "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_req_list)
{
    TEST_SETUP_NOARGS(req_list);

    fail_unless(t.msgtype == UPK_REQ_LIST, "msgtype");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == 4, "payload_len");

    TEST_TEARDOWN;
}

END_TEST;


START_TEST(test_proto_req_status)
{
    TEST_SETUP(req_status, "some other service");

    fail_unless(t.msgtype == UPK_REQ_STATUS, "msgtype");
    fail_unless(t.svc_id_len == strlen("some other service"), "svc_id_len");
    fail_unless(strcmp(t.svc_id, "some other service") == 0, "svc_id");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == (4 + 4 + strlen("some other service")), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_req_subscribe)
{
    TEST_SETUP(req_subscribe, "another service", true);

    fail_unless(t.msgtype == UPK_REQ_SUBSCRIBE, "msgtype");
    fail_unless(t.all_svcs == true, "all_svcs");
    fail_unless(t.svc_id_len == strlen("another service"), "svc_id_len");
    fail_unless(strcmp(t.svc_id, "another service") == 0, "svc_id");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == (4 + 1 + 4 + strlen("another service")), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_req_unsubscribe)
{
    TEST_SETUP(req_unsubscribe, "another service", true);

    fail_unless(t.msgtype == UPK_REQ_UNSUBSCRIBE, "msgtype");
    fail_unless(t.all_svcs == true, "all_svcs");
    fail_unless(t.svc_id_len == strlen("another service"), "svc_id_len");
    fail_unless(strcmp(t.svc_id, "another service") == 0, "svc_id");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == (4 + 1 + 4 + strlen("another service")), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_req_disconnect)
{
    TEST_SETUP_NOARGS(req_disconnect);

    fail_unless(t.msgtype == UPK_REQ_DISCONNECT);

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == (4), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;


START_TEST(test_proto_repl_seq_start)
{
    TEST_SETUP(repl_seq_start, UPK_REPL_LISTING, 19);

    fail_unless(t.msgtype == UPK_REPL_SEQ_START, "msgtype");
    fail_unless(t.msg_seq_type == UPK_REPL_LISTING, "seq_msg_type");
    fail_unless(t.msg_seq_count == 19, "seq_msg_count");

    fail_unless(pkt->pkttype == PKT_REPLY, "pkttype");
    fail_unless(pkt->payload_len == (4 + 4 + 4), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_repl_seq_end)
{
    TEST_SETUP(repl_seq_end, true);

    fail_unless(t.msgtype == UPK_REPL_SEQ_END, "msgtype");
    fail_unless(t.commit == true, "commit");

    fail_unless(pkt->pkttype == PKT_REPLY, "pkttype");
    fail_unless(pkt->payload_len == (4 + 1), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_repl_result)
{
    TEST_SETUP(repl_result, "some message", true);

    fail_unless(t.msgtype == UPK_REPL_RESULT, "msgtype");
    fail_unless(t.successful == true, "successful");
    fail_unless(t.msg_len == strlen("some message"), "msg_len");
    fail_unless(strcmp(t.msg, "some message") == 0, "msg");

    fail_unless(pkt->pkttype == PKT_REPLY, "pkttype");
    fail_unless(pkt->payload_len == (4 + 1 + 4 + strlen("some message")), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_repl_listing)
{
    TEST_SETUP(repl_listing, "now with more serviceness");

    fail_unless(t.msgtype == UPK_REPL_LISTING, "msgtype");
    fail_unless(t.svc_id_len == strlen("now with more serviceness"), "svc_id_len");
    fail_unless(strcmp(t.svc_id, "now with more serviceness") == 0, "svc_id");

    fail_unless(pkt->pkttype == PKT_REPLY, "pkttype");
    fail_unless(pkt->payload_len == (4 + 4 + strlen("now with more serviceness")), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_repl_svcinfo)
{
#define action_name "svcinfo last action name"
#define service_name "svcinfo service"
    v0_svcinfo_t            svcinfo = {
        .last_action_time = 12345678,
        .last_action_status = 87654321,
        .last_action_name_len = strlen(action_name),
        .last_action_name = action_name,
        .last_signal_time = 98765432,
        .last_signal_status = 23456789,
        .last_signal_name = UPK_SIG_KILL,
        .buddy_pid = 1732,
        .proc_pid = 1733,
        .current_state = UPK_STATE_RUNNING,
        .prior_state = UPK_STATE_SHUTDOWN,
    };
    uint32_t                svcinfo_length = 0;

    TEST_SETUP(repl_svcinfo, service_name, &svcinfo);

    fail_unless(t.msgtype == UPK_REPL_SVCINFO, "msgtype");

    svcinfo_length += 4;                                   /* .last_action_time = 12345678, */
    svcinfo_length += 4;                                   /* .last_action_status = 87654321, */
    svcinfo_length += 4;                                   /* .last_action_name_len = strlen(action_name), */
    svcinfo_length += strlen(action_name);                 /* .last_action_name = action_name, */
    svcinfo_length += 4;                                   /* .last_signal_time = 98765432, */
    svcinfo_length += 4;                                   /* .last_signal_status = 23456789, */
    svcinfo_length += 4;                                   /* .last_signal_name = UPK_SIG_KILL, */
    svcinfo_length += 4;                                   /* .buddy_pid = 1732, */
    svcinfo_length += 4;                                   /* .proc_pid = 1733, */
    svcinfo_length += 4;                                   /* .current_state = UPK_STATE_RUNNING, */
    svcinfo_length += 4;                                   /* .prior_state = UPK_STATE_DOWN, */

    fail_unless(t.svcinfo_len == svcinfo_length, "t.svcinfo_len");
    fail_unless(t.svcinfo.last_action_time == 12345678, "svcinfo.last_action_time");
    fail_unless(t.svcinfo.last_action_status == 87654321, "svcinfo.last_action_status");
    fail_unless(t.svcinfo.last_action_name_len == strlen(action_name), "svcinfo.last_action_name_len");
    fail_unless(strcmp(t.svcinfo.last_action_name, action_name) == 0, "t.svcinfo.last_action_name");
    fail_unless(t.svcinfo.last_signal_time == 98765432, "t.svcinfo.last_signal_time");
    fail_unless(t.svcinfo.last_signal_status == 23456789, "t.svcinfo.last_signal_status");
    fail_unless(t.svcinfo.last_signal_name == UPK_SIG_KILL, "t.svcinfo.last_signal_name");
    fail_unless(t.svcinfo.buddy_pid == 1732, "t.svcinfo.buddy_pid");
    fail_unless(t.svcinfo.proc_pid == 1733, "t.svcinfo.proc_pid");
    fail_unless(t.svcinfo.current_state == UPK_STATE_RUNNING, "t.svcinfo.current_state");
    fail_unless(t.svcinfo.prior_state == UPK_STATE_SHUTDOWN, "t.svcinfo.prior_state");

    fail_unless(t.svc_id_len == strlen(service_name), "svc_id_len");
    fail_unless(strcmp(t.svc_id, service_name) == 0, "svc_id");

    fail_unless(pkt->pkttype == PKT_REPLY, "pkttype");
    fail_unless(pkt->payload_len == (4 + 4 + t.svcinfo_len + 4 + strlen(service_name)), "payload_len");

    TEST_TEARDOWN;
#undef action_name
#undef service_name
}

END_TEST;

START_TEST(test_proto_repl_ack)
{
    TEST_SETUP_NOARGS(repl_ack);

    fail_unless(t.msgtype == UPK_REPL_ACK, "msgtype");

    fail_unless(pkt->pkttype == PKT_REPLY, "pkttype");
    fail_unless(pkt->payload_len == (4), "payload_len");

    TEST_TEARDOWN;
}
END_TEST;

START_TEST(test_proto_repl_error)
{
#define service_name  "service on which error is being reported"
#define errmsg "Some helpful error message telling what happened"
    TEST_SETUP(repl_error, service_name, UPK_ERR_UNKNOWN, errmsg, UPK_ERRLVL_ERROR);

    fail_unless(t.msgtype == UPK_REPL_ERROR, "msgtype");
    fail_unless(t.errlevel == UPK_ERRLVL_ERROR, "errlevel");
    check_isequal(t.uerrno, UPK_ERR_UNKNOWN, %d);
    
    
    fail_unless(t.msg_len == strlen(errmsg), "msg_len");
    fail_unless(strcmp(t.msg, errmsg) == 0, "msg");
    fail_unless(t.svc_id_len == strlen(service_name), "svc_id_len");
    fail_unless(strcmp(t.svc_id, service_name) == 0, "svc_id");

    fail_unless(pkt->pkttype == PKT_REPLY, "pkttype");
    fail_unless(pkt->payload_len == (4 + 4 + 4 + 4 + strlen(errmsg) + 4 + strlen(service_name)), "payload_len");

    TEST_TEARDOWN;
#undef service_name
#undef errmsg
}
END_TEST;

START_TEST(test_proto_pub_publication)
{
    TEST_SETUP_NOARGS(pub_publication);

    fail_unless(t.msgtype == UPK_PUB_PUBLICATION, "msgtype");

    fail_unless(pkt->pkttype == PKT_PUBMSG, "pkttype");
    fail_unless(pkt->payload_len == (4), "payload_len");

    TEST_TEARDOWN;
}
END_TEST;

START_TEST(test_proto_pub_cancelation)
{
    TEST_SETUP_NOARGS(pub_cancelation);

    fail_unless(t.msgtype == UPK_PUB_CANCELATION, "msgtype");

    fail_unless(pkt->pkttype == PKT_PUBMSG, "pkttype");
    fail_unless(pkt->payload_len == (4), "payload_len");

    TEST_TEARDOWN;
}
END_TEST;


TCase                  *
v0_proto_requests(void)
{
    TCase                  *requests = tcase_create("requests");

#ifdef CHECK_PREAMBLE
    tcase_add_test(requests, test_proto_req_preamble);
    tcase_add_test(requests, test_proto_repl_preamble);
#endif
    tcase_add_test(requests, test_proto_req_seq_start);
    tcase_add_test(requests, test_proto_req_seq_end);
    tcase_add_test(requests, test_proto_req_action);
    tcase_add_test(requests, test_proto_req_signal);
    tcase_add_test(requests, test_proto_req_list);
    tcase_add_test(requests, test_proto_req_status);
    tcase_add_test(requests, test_proto_req_subscribe);
    tcase_add_test(requests, test_proto_req_unsubscribe);
    tcase_add_test(requests, test_proto_req_disconnect);

    return requests;
}

TCase                  *
v0_proto_replies(void)
{
    TCase                  *replies = tcase_create("replies");

    tcase_add_test(replies, test_proto_repl_seq_start);
    tcase_add_test(replies, test_proto_repl_seq_end);
    tcase_add_test(replies, test_proto_repl_result);
    tcase_add_test(replies, test_proto_repl_listing);
    tcase_add_test(replies, test_proto_repl_svcinfo);
    tcase_add_test(replies, test_proto_repl_ack);
    tcase_add_test(replies, test_proto_repl_error);

    return replies;
}

TCase                  *
v0_proto_pubmsgs(void)
{
    TCase                  *pubmsgs = tcase_create("pubmsgs");

    tcase_add_test(pubmsgs, test_proto_pub_publication);
    tcase_add_test(pubmsgs, test_proto_pub_cancelation);

    return pubmsgs;
}

