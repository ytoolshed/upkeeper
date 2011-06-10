#include "std_include.h"

typedef enum {
    UPK_SVC_LOG_CAPTURE_STDOUT = 1,
    UPK_SVC_LOG_CAPTURE_STDERR = 2,
    UPK_SVC_LOG_SYSLOG_STDOUT = 4,
    UPK_SVC_LOG_SYSLOG_STDERR = 8,
    UPK_SVC_LOG_STDOUT_FILE = 16,
    UPK_SVC_LOG_STDERR_FILE = 32
} upk_svc_logopts_t;

typedef struct _upk_svc_desc upk_svc_desc_t;
struct _upk_svc_desc {
    unsigned char           svc_name[UPK_MAX_STRING_LEN];
    unsigned char           pkg[UPK_MAX_STRING_LEN];
    upk_svc_logopts_t       logopts;
    unsigned char           stdout_file_link[UPK_MAX_STRING_LEN];
    unsigned char           stderr_file_link[UPK_MAX_STRING_LEN];
    upk_svc_desc_t         *next;
};

typedef struct {
    unsigned char           svc_config_path[UPK_MAX_STRING_LEN];
    unsigned char           svc_run_path[UPK_MAX_STRING_LEN];
    unsigned char           controller_socket[UPK_MAX_STRING_LEN];
    unsigned char           default_log_path[UPK_MAX_STRING_LEN];
    unsigned char           default_stderr_logpath[UPK_MAX_STRING_LEN];
    unsigned char           default_stdout_logpath[UPK_MAX_STRING_LEN];
    upk_svc_logopts_t       default_logopts;
    upk_svc_desc_t         *svclist;
} upk_controller_config_t;
