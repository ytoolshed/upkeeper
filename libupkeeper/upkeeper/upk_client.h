
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

#ifndef _UPK_CLIENT_H
#define _UPK_CLIENT_H

#include "upk_types.h"

extern bool             upkeeper_action_request(char *service_id, char *action_name);
extern bool             upkeeper_signal_request(char *service_id, upk_signal_t signal);
extern upk_svcid_t     *upkeeper_list_services(void);

extern upk_svcinfo_t    upkeeper_get_status(char *service_id);

extern bool             upkeeper_subscribe(const char *svc_id);
extern bool             upkeeper_subscribe_all(void);

extern char            *upk_clientid(void);


#endif
