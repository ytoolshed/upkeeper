START_TEST(test_proto_req_seq_start)
{
    TEST_SETUP(req_seq_start, REQ_LIST, 15);

    fail_unless(t.msgtype == REQ_SEQ_START, "msgtype");
    fail_unless(t.msg_seq_type == REQ_LIST, "msg_seq_type");
    fail_unless(t.msg_seq_count == 15, "msg_seq_count");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == (4 + 4 + 4), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_req_seq_end)
{
    TEST_SETUP(req_seq_end, true);

    fail_unless(t.msgtype == REQ_SEQ_END, "msgtype");
    fail_unless(t.commit == true, "commit");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == (4 + 1), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_action_req)
{
    TEST_SETUP(action_req, "some service", "some action");

    fail_unless(t.msgtype == REQ_ACTION, "msgtype");
    fail_unless(t.svc_id_len == strlen("some service"), "svc_id_len");
    fail_unless(strcmp(t.svc_id, "some service") == 0, "svc_id");
    fail_unless(t.action_len == strlen("some action"), "action_len");
    fail_unless(strcmp(t.action, "some action") == 0, "action");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == 4 + 4 + strlen("some service") + 4 + strlen("some action"), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_signal_req)
{
    TEST_SETUP(signal_req, "some service", UPK_SIG_KILL, true, true);


    fail_unless(t.msgtype == REQ_SIGNAL, "msgtype");
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

START_TEST(test_proto_list_req)
{
    TEST_SETUP_NOARGS(list_req);

    fail_unless(t.msgtype == REQ_LIST, "msgtype");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == 4, "payload_len");

    TEST_TEARDOWN;
}

END_TEST;


START_TEST(test_proto_status_req)
{
    TEST_SETUP(status_req, "some other service");

    fail_unless(t.msgtype == REQ_STATUS, "msgtype");
    fail_unless(t.svc_id_len == strlen("some other service"), "svc_id_len");
    fail_unless(strcmp(t.svc_id, "some other service") == 0, "svc_id");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == (4 + 4 + strlen("some other service")), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_subscr_req)
{
    TEST_SETUP(subscr_req, "another service", true);

    fail_unless(t.msgtype == REQ_SUBSCRIBE, "msgtype");
    fail_unless(t.all_svcs == true, "all_svcs");
    fail_unless(t.svc_id_len == strlen("another service"), "svc_id_len");
    fail_unless(strcmp(t.svc_id, "another service") == 0, "svc_id");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == (4 + 1 + 4 + strlen("another service")), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_unsubs_req)
{
    TEST_SETUP(unsubs_req, "another service", true);

    fail_unless(t.msgtype == REQ_UNSUBSCRIBE, "msgtype");
    fail_unless(t.all_svcs == true, "all_svcs");
    fail_unless(t.svc_id_len == strlen("another service"), "svc_id_len");
    fail_unless(strcmp(t.svc_id, "another service") == 0, "svc_id");

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == (4 + 1 + 4 + strlen("another service")), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_discon_req)
{
    TEST_SETUP_NOARGS(discon_req);

    fail_unless(t.msgtype == REQ_DISCONNECT);

    fail_unless(pkt->pkttype == PKT_REQUEST, "pkttype");
    fail_unless(pkt->payload_len == (4), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;


START_TEST(test_proto_repl_seq_start)
{
    TEST_SETUP(repl_seq_start, REPL_LISTING, 19);

    fail_unless(t.msgtype == REPL_SEQ_START, "msgtype");
    fail_unless(t.msg_seq_type == REPL_LISTING, "seq_msg_type");
    fail_unless(t.msg_seq_count == 19, "seq_msg_count");

    fail_unless(pkt->pkttype == PKT_REPLY, "pkttype");
    fail_unless(pkt->payload_len == (4 + 4 + 4), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_repl_seq_end)
{
    TEST_SETUP(repl_seq_end, true);

    fail_unless(t.msgtype == REPL_SEQ_END, "msgtype");
    fail_unless(t.commit == true, "commit");

    fail_unless(pkt->pkttype == PKT_REPLY, "pkttype");
    fail_unless(pkt->payload_len == (4 + 1), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_result_repl)
{
    TEST_SETUP(result_repl, "some message", true);

    fail_unless(t.msgtype == REPL_RESULT, "msgtype");
    fail_unless(t.successful == true, "successful");
    fail_unless(t.msg_len == strlen("some message"), "msg_len");
    fail_unless(strcmp(t.msg, "some message") == 0, "msg");

    fail_unless(pkt->pkttype == PKT_REPLY, "pkttype");
    fail_unless(pkt->payload_len == (4 + 1 + 4 + strlen("some message")), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_listing_repl)
{
    TEST_SETUP(listing_repl, "now with more serviceness");

    fail_unless(t.msgtype == REPL_LISTING, "msgtype");
    fail_unless(t.svc_id_len == strlen("now with more serviceness"), "svc_id_len");
    fail_unless(strcmp(t.svc_id, "now with more serviceness") == 0, "svc_id");

    fail_unless(pkt->pkttype == PKT_REPLY, "pkttype");
    fail_unless(pkt->payload_len == (4 + 4 + strlen("now with more serviceness")), "payload_len");

    TEST_TEARDOWN;
}

END_TEST;

START_TEST(test_proto_svcinfo_repl)
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

    TEST_SETUP(svcinfo_repl, service_name, &svcinfo);

    fail_unless(t.msgtype == REPL_SVCINFO, "msgtype");

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

START_TEST(test_proto_ack_repl)
{
    TEST_SETUP_NOARGS(ack_repl);

    fail_unless(t.msgtype == REPL_ACK, "msgtype");

    fail_unless(pkt->pkttype == PKT_REPLY, "pkttype");
    fail_unless(pkt->payload_len == (4), "payload_len");

    TEST_TEARDOWN;
}
END_TEST;

START_TEST(test_proto_error_repl)
{
#define service_name  "service on which error is being reported"
#define errmsg "Some helpful error message telling what happened"
    TEST_SETUP(error_repl, service_name, errmsg, UPK_ERRLVL_ERROR);

    fail_unless(t.msgtype == REPL_ERROR, "msgtype");
    fail_unless(t.errlevel == UPK_ERRLVL_ERROR, "errlevel");
    fail_unless(t.msg_len == strlen(errmsg), "msg_len");
    fail_unless(strcmp(t.msg, errmsg) == 0, "msg");
    fail_unless(t.svc_id_len == strlen(service_name), "svc_id_len");
    fail_unless(strcmp(t.svc_id, service_name) == 0, "svc_id");

    fail_unless(pkt->pkttype == PKT_REPLY, "pkttype");
    fail_unless(pkt->payload_len == (4 + 4 + 4 + strlen(errmsg) + 4 + strlen(service_name)), "payload_len");

    TEST_TEARDOWN;
#undef service_name
#undef errmsg
}
END_TEST;

START_TEST(test_proto_pub_pubmsg)
{
    TEST_SETUP_NOARGS(pub_pubmsg);

    fail_unless(t.msgtype == PUB_PUBLICATION, "msgtype");

    fail_unless(pkt->pkttype == PKT_PUBMSG, "pkttype");
    fail_unless(pkt->payload_len == (4), "payload_len");

    TEST_TEARDOWN;
}
END_TEST;

START_TEST(test_proto_cancel_pubmsg)
{
    TEST_SETUP_NOARGS(cancel_pubmsg);

    fail_unless(t.msgtype == PUB_CANCELATION, "msgtype");

    fail_unless(pkt->pkttype == PKT_PUBMSG, "pkttype");
    fail_unless(pkt->payload_len == (4), "payload_len");

    TEST_TEARDOWN;
}
END_TEST;


TCase                  *
v0_proto_requests(void)
{
    TCase                  *requests = tcase_create("requests");

    tcase_add_test(requests, test_proto_req_seq_start);
    tcase_add_test(requests, test_proto_req_seq_end);
    tcase_add_test(requests, test_proto_action_req);
    tcase_add_test(requests, test_proto_signal_req);
    tcase_add_test(requests, test_proto_list_req);
    tcase_add_test(requests, test_proto_status_req);
    tcase_add_test(requests, test_proto_subscr_req);
    tcase_add_test(requests, test_proto_unsubs_req);
    tcase_add_test(requests, test_proto_discon_req);

    return requests;
}

TCase                  *
v0_proto_replies(void)
{
    TCase                  *replies = tcase_create("replies");

    tcase_add_test(replies, test_proto_repl_seq_start);
    tcase_add_test(replies, test_proto_repl_seq_end);
    tcase_add_test(replies, test_proto_result_repl);
    tcase_add_test(replies, test_proto_listing_repl);
    tcase_add_test(replies, test_proto_svcinfo_repl);
    tcase_add_test(replies, test_proto_ack_repl);
    tcase_add_test(replies, test_proto_error_repl);

    return replies;
}

TCase                  *
v0_proto_pubmsgs(void)
{
    TCase                  *pubmsgs = tcase_create("pubmsgs");

    tcase_add_test(pubmsgs, test_proto_pub_pubmsg);
    tcase_add_test(pubmsgs, test_proto_cancel_pubmsg);

    return pubmsgs;
}

