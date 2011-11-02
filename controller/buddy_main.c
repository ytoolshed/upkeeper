
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

#include "buddy.h"
#include <getopt.h>
#include <stdio.h>
#include <errno.h>

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static void
buddy_usage(char *progname)
{
    char                    usage_txt[][128] = {
        "  -R NUM, --ringbuffer_size=NUM",
        "     specify the size of the fixed-allocation ringbuffer in the buddy",
        "  -r NUM, --retries=NUM",
        "     for cases where the buddy tries to phone home to update status, such as when the buddy",
        "     itself is terminated via signal, specify the number of times to try to contact the",
        "     controller to provide report the contents of its ringbuffer.",
        "  -u UID, --setuid",
        "     setuid",
        "  -g GID, --setgid",
        "     setgid",
        "  -q, --quite",
        "     Decrease verbosity",
        "  -v, --verbose",
        "     Increase verbosity",
        "  -V, --version",
        "     Print version and exit",
        "",
    };
    size_t                  n = 0;

    printf("Usage: %s [ -R NUM | --ringbuffer_size=NUM ] [ -r NUM | --retries=NUM ] \n\
           [ -u | --setuid UID ] [-g | --setgid ] \n\
           [ -q | --quite] [-v | --verbose] [ -V | --version ] \n\
           service-name\n\n", progname);
    for(n = 0; n < sizeof(usage_txt) / 128; n++) {
        printf("%s\n", usage_txt[n]);
    }
    return;
}


/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static inline           bool
numeric_string(const char *string, long *num)
{
    char                   *p = NULL;
    uint8_t                 base = 10;

    if(strncmp(string, "0", 1) == 0)
        base = 8;
    if(strncmp(string, "0x", 2) == 0)
        base = 16;

    errno = 0;
    *num = strtoul(string, &p, base);
    if(p == string)
        return false;

    if((errno == ERANGE && (*num == LONG_MAX || *num == LONG_MIN)) || (*num <= 0))
        return false;

    return true;
}


/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static int
opt_parse(int argc, char **argv, char **envp)
{
    int                     c = 0, option_index = 0;
    char                    cbuf[3] = "-";
    const char             *p, pbuf[1] = "";
    long                    num = 0;
    size_t                  len = 0;
    bool                    valid = true;
    struct option           long_options[] = {
        {"quiet", 0, 0, 'q'},
        {"verbose", 0, 0, 'v'},
        {"version", 0, 0, 'V'},
        {"help", 0, 0, 'h'},
        {"ringbuffer_size", 1, 0, 'R'},
        {"retries", 1, 0, 'r'},
        {"setuid", 1, 0, 'u'},
        {"setgid", 1, 0, 'g'},
        {"buddy_path", 1, 0, 0},
        {"buddy_uuid", 1, 0, 0},
        {0, 0, 0, 0}
    };

    while((c = getopt_long(argc, argv, "qhvR:r:u:g:", long_options, &option_index)) >= 0) {
        *(cbuf + 1) = (option_index) ? '-' : c;
        p = (option_index) ? long_options[option_index].name : pbuf;

        switch (c) {
        case 0:
            if(strncmp("buddy_path", long_options[option_index].name, strlen("buddy_path")) == 0) {
                strncpy(buddy_root_path, optarg, sizeof(buddy_root_path) - 1);
                break;
            } else if(strncmp("buddy_uuid", long_options[option_index].name, strlen("buddy_uuid")) == 0) {
                if(is_valid_upk_uuid_string(optarg)) {
                    upk_string_to_uuid(&buddy_uuid, optarg);
                    break;
                }
            }
            valid = false;
            break;
        case 'q':
            upk_diag_verbosity--;
            break;
        case 'v':
            upk_diag_verbosity++;
            break;
        case 'R':
            if(!(valid = numeric_string(optarg, &num)))
                printf("%s: option `%s%s' requires a numeric argument\n", argv[0], cbuf, p);
            else
                ringbuffer_size = num;
            break;
        case 'r':
            if(!(valid = numeric_string(optarg, &num)))
                printf("%s: option `%s%s' requires a numeric argument\n", argv[0], cbuf, p);
            else
                reconnect_retries = num;
            break;
        case 'u':
            if(!(valid = numeric_string(optarg, &num)))
                printf("%s: option `%s%s' requires a numeric argument\n", argv[0], cbuf, p);
            else
                buddy_setuid = num;
            break;
        case 'g':
            if(!(valid = numeric_string(optarg, &num)))
                printf("%s: option `%s' requires a numeric argument\n", argv[0], argv[optind]);
            else
                buddy_setgid = num;
            break;
        case ':':
            printf("%s: option `%s' requires an argument\n", argv[0], argv[optind - 1]);
            valid = false;
            break;
        default:
            valid = false;
            break;
        }
    }

    if(optind < argc && memcmp(argv[optind], "--", 3) == 0)
        optind++;

    if(optind == argc) {
        printf("%s: must specify name of service to start\n", argv[0]);
        valid = false;
    } else {
        len = strnlen(argv[optind], BUDDY_MAX_STRING_LEN);
        buddy_service_name = calloc(1, len + 1);
        strncpy(buddy_service_name, argv[optind], len);
    }

    /* if(strnlen(buddy_uuid, 37) != 36) valid = false; */

    if(!valid)
        buddy_usage(argv[0]);

    return valid;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static void
buddy_diag_handler(upk_diaglvl_t diaglvl, const char *label, const char *loc, const char *fmt, va_list ap)
{
    static bool             show_label, show_loc;

    show_label = show_loc = false;

    if(diaglvl < UPK_DIAGLVL_ERROR || diaglvl > UPK_DIAGLVL_INFO)
        show_loc = true;

    if(upk_diag_verbosity > UPK_DIAGLVL_INFO)
        show_loc = show_label = true;

    switch (diaglvl) {
    case UPK_DIAGLVL_FATAL:
    case UPK_DIAGLVL_ALERT:
    case UPK_DIAGLVL_CRIT:
    case UPK_DIAGLVL_ERROR:
        show_label = true;
    default:
        if(strlen(loc) > 0 && show_loc)
            fprintf(stderr, "%s: ", loc);
        if(strlen(label) > 0 && show_label)
            fprintf(stderr, "%s: ", label);
        vfprintf(stderr, fmt, ap);
        break;
    }
}

extern
/* ********************************************************************************************************************
   ******************************************************************************************************************* */
    int
main(int argc, char **argv, char **envp)
{
    int                     retval = 0;

    ringbuffer_size = 32;
    upk_diag_verbosity = 5;

    chdir("/");
    /* fclose(stdin); fclose(stderr); */

    if((retval = (int) opt_parse(argc, argv, envp))) {
        proc_envp = envp;
        buddy_init(buddy_diag_handler);
        retval = buddy_event_loop();
        buddy_cleanup();
    }
    return retval;
}
