
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

#include "controller.h"
#include "ctrl_protocol.h"
#include <stdio.h>
#include <errno.h>

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
int
main(int argc, char **argv, char **envp)
{
    int32_t                 sock_fd;

    upk_diag_verbosity = UPK_DIAGLVL_DEBUG1;
    upk_ctrl_init();

    UPKLIST_FOREACH(upk_runtime_configuration.svclist) {
        create_buddy(upk_runtime_configuration.svclist->thisp);
    }

    fclose(stdin);                                         /* fclose(stdout); fclose(stderr); */
    chdir("/");

    sock_fd = ctrl_sock_setup();
    if(sock_fd >= 0) {
        event_loop(sock_fd);
    }
    upk_ctrl_free_config();

    return 0;
}
