#ifndef _UPK_NETWORK_H
#define _UPK_NETWORK_H

#include "upk_include.h"

/**
  @brief this is a tailq for network message queuing 
  */
typedef struct _upk_net_event_queue upk_net_event_queue_t;
/**
  @brief this is a tailq for network message queuing 
  */
struct _upk_net_event_queue {
	upk_pkt_buf_t *msg; /*<! serialized packet */
	upk_net_event_queue_t * next; /*<! next item */
};

typedef UPKLIST_METANODE(upk_net_event_queue_t, upk_net_event_queue_listhead_p), upk_net_event_queue_listhead_t;

/* upkeeper/upk_network.c */
extern int upk_client_conn_fd;
extern int upk_neg_protocol;

/**
  @brief make unix-domain socket connection.

  @param[in] sockpath   the path to the socket to connect to (e.g. the controller socket)
  @return the file descriptor of the resulting connection
  @return -2 unable to connect, check errno
 */


extern int upk_client_connect(const char *sockpath);

/**
  @brief retry socket connection, pausing between each retry

  @param[in] retries    number of times to retry
  @param[in] delay      duration to wait between retry attempts
  @param[in] sockpath   path to the socket to connect to (e.g. the controller socket)

  @return the file descriptor of the resulting connection
  @return -2 unable to connect, check errno
 */
extern int upk_client_retry(int retries, struct timespec delay, const char *sockpath);

#endif
