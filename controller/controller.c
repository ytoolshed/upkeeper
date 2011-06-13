#include "controller.h"
#include <stdio.h>
#include <errno.h>


/* *******************************************************************************************************************
 * ****************************************************************************************************************** */
int
main(int argc, char **argv, char **envp)
{
    upk_controller_config_t stub = {
        .svc_run_path = "/var/run/upkeeper/buddy/",
        .svc_config_path = "/etc/upkeeper.d",
        .controller_socket = "/var/run/upkeeper/controller.sock",
        .default_log_path = "/var/log/upkeeper",
        .default_stderr_logpath = "$logpath/%s.err",
        .default_stdout_logpath = "$logpath/%s.out",
        .default_logopts =
            UPK_SVC_LOG_CAPTURE_STDOUT | UPK_SVC_LOG_CAPTURE_STDERR | UPK_SVC_LOG_STDOUT_FILE | UPK_SVC_LOG_STDERR_FILE,
        .svclist = NULL,
    };
    int32_t                 sock_fd;

    fclose(stdin);                                          /* close(stdout); close(stderr); */
    chdir("/");

    sock_fd = ctrl_sock_setup(&stub);
    if(sock_fd >= 0) {
        event_loop(sock_fd);
    }

    return 0;
}
