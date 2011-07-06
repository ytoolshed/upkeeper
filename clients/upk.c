#include <upkeeper.h>
#include <upkeeper/upk_json.h>
#include <getopt.h>

static struct option longopts[] = {
    {"configfile", 1, 0, 0},
    {"name", 1, 0, 0}, 
    {"package", 1, 0, 0},
    {"provides", 1, 0, 0},
    {"uuid", 1, 0, 0},
    {"short_description", 1, 0, 0},
    {"long_description", 1, 0, 0},
    {"prerequisites", 1, 0, 0},
    {"start_priority", 1, 0, 0},
    {"buddy_shutdown_timeout", 1, 0, 0},
    {"kill_timeout", 1, 0, 0},
    {"max_consecutive_failures", 1, 0, 0},
    {"user_max_restarts", 1, 0, 0},
    {"user_restart_window", 1, 0, 0},
    {"user_rate_limit", 1, 0, 0},
    {"randomize_ratelimit", 1, 0, 0},
    {"setuid", 1, 0, 0},
    {"setgid", 1, 0, 0},
    {"ringbuffer_size", 1, 0, 0},
    {"reconnect_retries", 1, 0, 0},
    {"exec_start", 1, 0, 0},
    {"start_script", 1, 0, 0},
    {"stop_script", 1, 0, 0},
    {"reload_script", 1, 0, 0},
    {"custom_action_name", 1, 0, 0},
    {"custom_action_script", 1, 0, 0},
    {"pipe_stdout_script", 1, 0, 0},
    {"pipe_stderr_script", 1, 0, 0},
    {"redirect_stdout", 1, 0, 0},
    {"redirect_stderr", 1, 0, 0},
    {"initial_state", 1, 0, 0},
    {"prefer_buddy_state_for_stopped", 1, 0, 0},
    {"prefer_buddy_state_for_running", 1, 0, 0},
    {0, 0, 0, 0},
};


int
main(int argc, char ** argv, char ** envp)
{
    char c, cbuf[3] = "";
    const char *p, pbuf[1]="";
    int option_index;

    while((c = getopt_long(argc, argv, "h", longopts, &option_index)) > 0) {
        *(cbuf + 1) = (option_index) ? '-' : c;
        p = (option_index) ? longopts[option_index].name : pbuf;
    }

    upk_load_config();
    printf("%s", upk_default_configuration_vec);
}



