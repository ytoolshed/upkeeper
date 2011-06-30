#include "controller.h"
#include "ctrl_protocol.h"
#include <stdio.h>
#include <errno.h>

/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
int
main(int argc, char **argv, char **envp)
{
    upk_controller_config_t stub = {
        .StateDir = "/var/run/upkeeper",
        .SvcRunPath = "/var/run/upkeeper/buddy/",
        .SvcConfigPath = "/etc/upkeeper.d",
    };
    int32_t                 sock_fd;
    UPKLIST_METANODE(buddylist_t, blist);

    strncpy((char *)stub.controller_socket, (char *)stub.StateDir, UPK_MAX_STRING_LEN - 1);
    strcat((char *)stub.controller_socket, "/.controler.sock");

    strncpy((char *)stub.controller_buddy_sock, (char *)stub.StateDir, UPK_MAX_STRING_LEN - 1);
    strcat((char *)stub.controller_buddy_sock, "/.buddy.sock");

    upk_ctrl_config = &stub;

    blist = buddy_list();

    UPKLIST_FOREACH(blist) {
        create_buddy(blist->thisp);
    } 


    fclose(stdin);                                          /* close(stdout); close(stderr); */
    chdir("/");

    sock_fd = ctrl_sock_setup();
    if(sock_fd >= 0) {
        event_loop(sock_fd);
    }

    return 0;
}
