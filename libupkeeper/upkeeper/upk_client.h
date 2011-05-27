#ifndef _UPK_CLIENT_H
#define _UPK_CLIENT_H

#include "types.h"

bool upkeeper_action(unsigned char * service_id, unsigned char * action_name);
bool upkeeper_signal(unsigned char * service_id, upk_signal_t signal);

upk_svclist_t * upkeeper_list_services();
upk_svcinfo_t upkeeper_get_status(unsigned char * service_id);

#endif