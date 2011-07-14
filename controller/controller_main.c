#include "controller.h"
#include "ctrl_protocol.h"
#include <stdio.h>
#include <errno.h>

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
int
main(int argc, char **argv, char **envp)
{
    int32_t                 sock_fd;

    upk_diag_verbosity = UPK_DIAGLVL_DEBUG1;
    upk_ctrl_init();

    UPKLIST_FOREACH(upk_runtime_configuration.svclist) {
        create_buddy(upk_runtime_configuration.svclist->thisp);
    } 

    fclose(stdin);                                          /* close(stdout); close(stderr); */
    chdir("/");

    sock_fd = ctrl_sock_setup();
    if(sock_fd >= 0) {
        event_loop(sock_fd);
    }

    return 0;
}
