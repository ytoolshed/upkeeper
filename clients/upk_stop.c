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
    upk_conn_handle_meta_t *ctrl = NULL; 

    upk_diag_verbosity = UPK_DIAGLVL_DEBUG1;
    upk_ctrl_load_config();

    if(argc != 2)
        upk_fatal("Must provide service id\n");

    svc_id = argv[1];
    
    ctrl = upk_clnet_ctrl_connect();
    if(ctrl->thisp)  {
        upk_clnet_req_action(ctrl, svc_id, "stop");
    }
    upk_clnet_ctrl_disconnect(ctrl);
    upk_ctrl_free_config();
    return 0;
}
