#ifndef _UPK_ERROR_H
#define _UPK_ERROR_H

#include "types.h"

#define UPK_ERRMSG __upk_err_buf

#define UPK_ERR_INIT \
    upk_error_t __upk_error_type = 0; \
    char UPK_ERRMSG[UPK_MAX_STRING_LEN] = {0}

#define UPK_FUNC_ASSERT(A,B) do { if(! (A)) { __upk_error_type = B; goto __upk_err; } } while(0)
#define UPK_FUNC_ASSERT_MSG(A,B,...) \
    do { if(! (A)) { snprintf(UPK_ERRMSG, UPK_MAX_STRING_LEN, __VA_ARGS__); __upk_error_type = B; goto __upk_err; } } while(0)

#define IF_UPK_ERROR __upk_err: \
    if(*UPK_ERRMSG && __upk_error_type) upk_report_error_msg(__upk_error_type, UPK_ERRMSG); \
    else if(__upk_error_type) upk_report_error(__upk_error_type); \
    if(__upk_error_type)

typedef void (*err_rpt_callback_t)(upk_error_t);
typedef void (*err_rpt_msg_callback_t)(upk_error_t, unsigned const char *);

extern void upk_report_error(upk_error_t);
extern void upk_report_error_msg(upk_error_t, unsigned const char * msg);
extern const unsigned char * upk_strerror(upk_error_t err);
extern err_rpt_callback_t upk_reg_error_callback(err_rpt_callback_t);
extern err_rpt_msg_callback_t upk_reg_err_msg_callback(err_rpt_msg_callback_t);

#endif
