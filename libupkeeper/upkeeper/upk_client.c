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

#include "upk_include.h"

/**
  @file
  @brief public client API.
  */

/**
  @brief fill buf with the client_id string.

  @param[out] buf allocated buffer to write clientid into
  */
char *
upk_clientid(void)
{
    static char client_id[UPK_MAX_STRING_LEN] = "";

    if(*client_id == '\0')
        snprintf(client_id, UPK_MAX_STRING_LEN - 1, "%s-%d", upk_getprogname, getpid());
    return client_id;
}




/**
  @brief the body of the client event-loop; so you can call it once per pass of your own eventloop if you prefer.
  */
bool
upk_client_event_hook(void)
{
    return false;
}








