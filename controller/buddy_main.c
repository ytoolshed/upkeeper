#include "buddy.h"
#include <getopt.h>
#include <stdio.h>
#include <errno.h>

/* ********************************************************************************************************************
 * ******************************************************************************************************************* */
static void
buddy_usage(char *progname)
{
    char                    usage_txt[][128] = {
        "  -r NUM, --ringbuffer_size=NUM",
        "     specify the size of the fixed-allocation ringbuffer in the buddy",
        "  -q, --quite",
        "     Decrease verbosity",
        "  -v, --verbose",
        "     Increase verbosity",
        "  -V, --version",
        "     Print version and exit",
        "",
    };
    size_t                  n = 0;

    printf
        ("Usage: %s [ -r NUM | --ringbuffer_size=NUM ] [-q | --quite] [-v | --verbose] [-V | --version] service-name\n",
         progname);
    for(n = 0; n < sizeof(usage_txt) / 128; n++) {
        printf("%s\n", usage_txt[n]);
    }
    return;
}



/* ********************************************************************************************************************
 * ******************************************************************************************************************* */
static int
opt_parse(int argc, char **argv, char **envp)
{
    int                     c = 0, base = 0, option_index = 0;
    size_t len = 0;
    bool                    valid = true;
    char                   *buf = NULL;
    struct option           long_options[] = {
        {"quiet", 0, 0, 'q'},
        {"verbose", 0, 0, 'v'},
        {"version", 0, 0, 'V'},
        {"help", 0, 0, 'h'},
        {"ringbuffer_size", 1, 0, 'r'},
        {"setuid", 1, 0, 'u'},
        {"setgid", 1, 0, 'g'},
        {0, 0, 0, 0}
    };

    while((c = getopt_long(argc, argv, "qhvr:u:g:", long_options, &option_index)) > 0) {
        switch (c) {
        /* case 0:
            if(strncmp("buddy_path", long_options[option_index].name, strlen("buddy_path")) == 0) {
                strncpy(buddy_path, optarg, sizeof(buddy_path));
                break;
            }
            printf("%s: Unknown options: %s\n", argv[0], long_options[option_index].name);
            valid=false;
            break; */
        case 'q':
            verbose--;
            break;
        case 'v':
            verbose++;
            break;
        case 'r':
            if(strncmp(optarg, "0", 1) == 0)
                base = 8;
            if(strncmp(optarg, "0x", 2) == 0)
                base = 16;

            errno = 0;
            ringbuffer_size = strtol(optarg, &buf, base);
            if(buf == optarg) {
                printf("%s: `r' requires a numeric argument\n", argv[0]);
                valid = false;
            } else if((errno == ERANGE && (ringbuffer_size == LONG_MAX || ringbuffer_size == LONG_MIN))
                      || (ringbuffer_size <= 0)) {
                printf("%s: `r': invalid value: %d\n", argv[0], (int) ringbuffer_size);
                valid = false;
            }
            break;
        case 'u':
            /* FIXME: refactor */
            if(strncmp(optarg, "0", 1) == 0)
                base = 8;
            if(strncmp(optarg, "0x", 2) == 0)
                base = 16;

            errno = 0;
            buddy_uid = strtol(optarg, &buf, base);
        case 'g':
            if(strncmp(optarg, "0", 1) == 0)
                base = 8;
            if(strncmp(optarg, "0x", 2) == 0)
                base = 16;

            errno = 0;
            buddy_gid = strtol(optarg, &buf, base);
        default:
            printf("%s: unknown option: %c\n", argv[0], c);
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

    if(!valid)
        buddy_usage(argv[0]);

    return valid;
}

/* ********************************************************************************************************************
 * ******************************************************************************************************************* */
int
main(int argc, char **argv, char **envp)
{
    int                     retval = 0;

    ringbuffer_size = 32;

    if((retval = (int) opt_parse(argc, argv, envp))) {
        buddy_init();
        retval = buddy_event_loop();
        buddy_cleanup();
    }
    return retval;
}
