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

#endif
