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
    {"exec_stop", 1, 0, 0},
    {"stop_script", 1, 0, 0},
    {"exec_reload", 1, 0, 0},
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
    char c, cbuf[3] = "", *cp;
    const char *p, pbuf[1]="";
    bool finalize = false;

    long nbuf = 0;
    bool bbuf = false;
    int option_index = 0;
    upk_json_data_output_opts_t opts = { .pad = " ", .indent = "    ", .sep = "\n", .suppress_null_values = false };
    upk_svc_desc_t svc = { 0 };

    upk_svc_desc_clear(&svc);

    while((c = getopt_long(argc, argv, "sch", longopts, &option_index)) >= 0) {
        *(cbuf + 1) = (option_index) ? '-' : c;
        p = (option_index) ? longopts[option_index].name : pbuf;
        switch(c) {
            case 0:
                if(strcmp(longopts[option_index].name, "configfile") == 0)
                    break;
                else if(strcmp(longopts[option_index].name, "name") == 0)
                    strcpy(svc.Name, optarg);
                else if(strcmp(longopts[option_index].name, "package") == 0)
                    strcpy(svc.Package, optarg);
                else if(strcmp(longopts[option_index].name, "uuid") == 0) {
                    upk_string_to_uuid(optarg, &svc.UUID);
                }
                else if(strcmp(longopts[option_index].name, "short_description") == 0)
                    strcpy(svc.ShortDescription, optarg);
                else if(strcmp(longopts[option_index].name, "long_description") == 0) {
                    svc.LongDescription = calloc(1, strlen(optarg) + 1);
                    strcpy(svc.LongDescription, optarg);
                }
                else if(strcmp(longopts[option_index].name, "prerequisites") == 0)
                    break;
                else if(strcmp(longopts[option_index].name, "start_priority") == 0) {
                    upk_numeric_string(optarg, &nbuf);
                    svc.StartPriority = nbuf;
                }
                else if(strcmp(longopts[option_index].name, "buddy_shutdown_timeout") == 0) {
                    upk_numeric_string(optarg, &nbuf);
                    svc.BuddyShutdownTimeout = nbuf;
                }
                else if(strcmp(longopts[option_index].name, "kill_timeout") == 0) {
                    upk_numeric_string(optarg, &nbuf);
                    svc.KillTimeout = nbuf;
                }
                else if(strcmp(longopts[option_index].name, "max_consecutive_failures") == 0) {
                    upk_numeric_string(optarg, &nbuf);
                    svc.MaxConsecutiveFailures = nbuf;
                }
                else if(strcmp(longopts[option_index].name, "user_max_restarts") == 0) {
                    upk_numeric_string(optarg, &nbuf);
                    svc.UserMaxRestarts = nbuf;
                }
                else if(strcmp(longopts[option_index].name, "user_restart_window") == 0) {
                    upk_numeric_string(optarg, &nbuf);
                    svc.UserRestartWindow = nbuf;
                }
                else if(strcmp(longopts[option_index].name, "user_rate_limit") == 0) {
                    upk_numeric_string(optarg, &nbuf);
                    svc.UserRateLimit = nbuf;
                }
                else if(strcmp(longopts[option_index].name, "randomize_ratelimit") == 0)
                    break;
                else if(strcmp(longopts[option_index].name, "setuid") == 0) {
                    upk_numeric_string(optarg, &nbuf);
                    svc.SetUID = nbuf;
                }
                else if(strcmp(longopts[option_index].name, "setgid") == 0) {
                    upk_numeric_string(optarg, &nbuf);
                    svc.SetGID = nbuf;
                }
                else if(strcmp(longopts[option_index].name, "ringbuffer_size") == 0) {
                    upk_numeric_string(optarg, &nbuf);
                    svc.RingbufferSize = nbuf;
                }
                else if(strcmp(longopts[option_index].name, "reconnect_retries") == 0) {
                    upk_numeric_string(optarg, &nbuf);
                    svc.ReconnectRetries = nbuf;
                }
                else if(strcmp(longopts[option_index].name, "exec_start") == 0)
                    strcpy(svc.ExecStart, optarg);
                else if(strcmp(longopts[option_index].name, "start_script") == 0) {
                    svc.StartScript = calloc(1, strlen(optarg) + 1);
                    strcpy(svc.StartScript, optarg);
                }
                else if(strcmp(longopts[option_index].name, "exec_stop") == 0)
                    strcpy(svc.ExecStop, optarg);
                else if(strcmp(longopts[option_index].name, "stop_script") == 0) {
                    svc.StopScript = calloc(1, strlen(optarg) + 1);
                    strcpy(svc.StopScript, optarg);
                }
                else if(strcmp(longopts[option_index].name, "exec_reload") == 0)
                    strcpy(svc.ExecReload, optarg);
                else if(strcmp(longopts[option_index].name, "reload_script") == 0) {
                    svc.ReloadScript = calloc(1, strlen(optarg) + 1);
                    strcpy(svc.ReloadScript, optarg);
                }
                else if(strcmp(longopts[option_index].name, "custom_action_name") == 0)
                    break;
                else if(strcmp(longopts[option_index].name, "custom_action_script") == 0)
                    break;
                else if(strcmp(longopts[option_index].name, "pipe_stdout_script") == 0)
                    break;
                else if(strcmp(longopts[option_index].name, "pipe_stderr_script") == 0)
                    break;
                else if(strcmp(longopts[option_index].name, "redirect_stdout") == 0)
                    break;
                else if(strcmp(longopts[option_index].name, "redirect_stderr") == 0)
                    break;
                else if(strcmp(longopts[option_index].name, "initial_state") == 0) {
                    if(strcasecmp(optarg, "RUNNING") == 0) {
                        svc.InitialState = UPK_STATE_RUNNING;
                    }
                    else if(strcasecmp(optarg, "STOPPED") == 0) {
                        svc.InitialState = UPK_STATE_STOPPED;
                    }
                }
                else if(strcmp(longopts[option_index].name, "prefer_buddy_state_for_stopped") == 0) {
                    if( upk_boolean_string(optarg, &bbuf) )
                        svc.PreferBuddyStateForStopped = bbuf;
                }
                else if(strcmp(longopts[option_index].name, "prefer_buddy_state_for_running") == 0) {
                    if( upk_boolean_string(optarg, &bbuf) )
                        svc.PreferBuddyStateForRunning = bbuf;
                }
                break;
            case 'c':
                opts.pad = "";
                opts.indent = "",
                opts.sep = "";
                break;
            case 's':
                opts.suppress_null_values = true;
                break;
            case 'f':
                finalize=true;
                break;
        }
    }

    /*
    if(finalize)
        UPKLIST_FOREACH(cfg.svclist) {
            UPKLIST_APPEND(svclist);
            upk_svc_desc_clear(svclist->thisp);
                                        upk_finalize_svc_desc(svclist->thisp, cfg.svclist->thisp);
                                            }
    */

    cp = upk_json_serialize_svc_config(&svc, opts);
    printf("%s", cp);
    free(cp);
}



