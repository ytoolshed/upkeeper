#ifndef _UPK_ERROR_H
#define _UPK_ERROR_H

#define UPK_ERR_INIT upk_error_t __upk_error_type = 0;
#define UPK_FUNC_ASSERT(A,B) do { if(! (A)) { __upk_error_type = B; goto __upk_err; } } while(0)
#define IF_UPK_ERROR __upk_err: \
    if(__upk_error_type) upk_report_error(__upk_error_type); \
    if(__upk_error_type)

typedef enum {
	UPK_ERR_UNKNOWN=1,
	UPK_ERR_UNSUP,
	UPK_ERR_INVALID_PKT,
} upk_error_t;

typedef uint32_t (*err_rpt_callback_t)(upk_error_t);
typedef uint32_t (*err_rpt_msg_callback_t)(upk_error_t, char *);

extern void upk_report_error(upk_error_t);
extern void upk_reg_error_callback(err_rpt_callback_t);
extern void upk_reg_err_msg_callback(err_rpt_msg_callback_t);
err_rpt_callback_t upk_get_err_callback(void);
err_rpt_msg_callback_t upk_get_err_msg_callback(void);

#endif
