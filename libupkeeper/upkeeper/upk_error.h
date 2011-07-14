#ifndef _UPK_ERROR_H
#define _UPK_ERROR_H

/**
  @file
  @brief handle errors and single-exit-point semantics.
  @defgroup upk_errors
  @{
  */

#include "upk_std_include.h"
#include <stdarg.h>

/* FIXME: Refactor most of this, ideally getting rid of the custom callback; and maybe just using the diag callback for 
   everything */

#define UPK_ERRMSG __upk_err_buf

#define UPK_ERR_INIT \
    upk_error_t __upk_error_type = 0; \
    char UPK_ERRMSG[UPK_MAX_STRING_LEN] = {0}

#define UPK_FUNC_ASSERT(A,B) do { if(! (A)) { __upk_error_type = B; goto __upk_err; } } while(0)
#define UPK_FUNC_ASSERT_MSG(A,B,...) \
    do { if(! (A)) { snprintf(UPK_ERRMSG, UPK_MAX_STRING_LEN, __VA_ARGS__); __upk_error_type = B; goto __upk_err; } } while(0)

#define IF_UPK_ERROR \
    __upk_err: \
    if(*UPK_ERRMSG && __upk_error_type) upk_report_error_msg(__upk_error_type, (unsigned char *) UPK_ERRMSG); \
    else if(__upk_error_type) upk_report_error(__upk_error_type); \
    if(__upk_error_type)


/* ************************************* */
/* These two items must be kept in sync: */
/* ************************************* */

#define __UPK_DIAG_LABELS \
    char                    upk_diag_label_idx[][16] = {\
        [UPK_DIAGLVL_FATAL]   = "FATAL",\
        [UPK_DIAGLVL_ALERT]   = "ALERT",\
        [UPK_DIAGLVL_CRIT]    = "CRITICAL",\
        [UPK_DIAGLVL_ERROR]   = "ERROR",\
        [UPK_DIAGLVL_WARN]    = "WARNING",\
        [UPK_DIAGLVL_NOTICE]  = "NOTICE",\
        [UPK_DIAGLVL_VERBOSE] = "VERBOSE",\
        [UPK_DIAGLVL_INFO]    = "INFO",\
        [UPK_DIAGLVL_DEBUG0]  = "DEBUG0",\
        [UPK_DIAGLVL_DEBUG1]  = "DEBUG1"\
    };


typedef enum {
    UPK_DIAGLVL_FATAL = 0,                                 /*!< fatal error, operation cannot continue */
    UPK_DIAGLVL_ALERT,                                     /*!< alert; something really bad happened, but we believe
                                                              we can continue */
    UPK_DIAGLVL_CRIT,                                      /*!< critical; something really bad happened, that probably 
                                                              shouldn't happen; we can continue */
    UPK_DIAGLVL_ERROR,                                     /*!< An error occurred. */
    UPK_DIAGLVL_WARN,                                      /*!< A warning, serious, but not too serious */
    UPK_DIAGLVL_NOTICE,                                    /*!< Something you might want to notice; possibly normal
                                                              operation */
    UPK_DIAGLVL_VERBOSE,                                   /*!< verbose, probably normal operation */
    UPK_DIAGLVL_INFO,                                      /*!< information, definitely normal operation, intended to
                                                              aid in troubleshooting user-configurable things */
    UPK_DIAGLVL_DEBUG0,                                    /*!< debug0, intended to help find bugs in the program; but 
                                                              may also help in finding configuration or environment
                                                              problems */
    UPK_DIAGLVL_DEBUG1                                     /*!< debug1, useful in finding and demonstrating bugs in
                                                              the program (you should probably be using gdb by this
                                                              point) */
} upk_diaglvl_t;


typedef enum {
    UPK_ERRLVL_ERROR,                                      /*!< semantically similar to diaglvl; but for
                                                              error-reporting; may become purely protocol */
} upk_errlevel_t;


/* ************************************* */
/* These two items must be kept in sync: */
/* ************************************* */
typedef enum {
    UPK_ERR_UNKNOWN = 0,                                   /*!< unknown error */
    UPK_ERR_UNSUP,                                         /*!< unsupported */
    UPK_ERR_INVALID_PKT,                                   /*!< invalid packet, possibly invalid protocol, packet
                                                              dimensions, or checksum */
    UPK_SOCKET_FAILURE,                                    /*!< unable to bind/connect/accept/listen on a socket */
    UPK_JSON_PARSE_ERROR,                                  /*!< json parse error; additional information should also
                                                              be provided */
} upk_error_t;


#define __UPK_ERRORS_DESC_ARRAY \
    const unsigned char     __upk_errors[][128] = { \
        "unknown", \
        "unsupported", \
        "invalid packet", \
        "socket failure", \
        "JSON parser error", \
    }


typedef void            (*err_rpt_callback_t) (upk_error_t);
typedef void            (*err_rpt_msg_callback_t) (upk_error_t, unsigned const char *);
typedef void            (*diag_output_callback_t) (upk_diaglvl_t diaglvl, const char *label, const char *loc,
                                                   const char *fmt, va_list ap);


/* upk_error.c */
extern int32_t          upk_diag_verbosity;
extern char             label_idx[][16];

extern const unsigned char *upk_strerror(upk_error_t err);
extern void             upk_report_error(upk_error_t err);
extern void             upk_report_error_msg(upk_error_t err, unsigned const char *msg);
extern err_rpt_callback_t upk_reg_error_callback(err_rpt_callback_t func);
extern err_rpt_msg_callback_t upk_reg_err_msg_callback(err_rpt_msg_callback_t func);
extern diag_output_callback_t upk_reg_diag_callback(diag_output_callback_t func);

extern int32_t          _upk_fatal(const char *loc, const char *fmt, ...);
extern int32_t          _upk_alert(const char *loc, const char *fmt, ...);
extern int32_t          _upk_crit(const char *loc, const char *fmt, ...);
extern int32_t          _upk_error(const char *loc, const char *fmt, ...);
extern int32_t          _upk_warn(const char *loc, const char *fmt, ...);
extern int32_t          _upk_notice(const char *loc, const char *fmt, ...);
extern int32_t          _upk_info(const char *loc, const char *fmt, ...);
extern int32_t          _upk_verbose(const char *loc, const char *fmt, ...);
extern int32_t          _upk_debug0(const char *loc, const char *fmt, ...);
extern int32_t          _upk_debug1(const char *loc, const char *fmt, ...);

#define _UPK_AS_STRING(A) #A
#define UPK_AS_STRING(A) _UPK_AS_STRING(A)
#define _UPK_DIAG_LOCATION __FILE__ ":" UPK_AS_STRING(__LINE__)

#define upk_fatal(...) _upk_fatal(_UPK_DIAG_LOCATION, __VA_ARGS__)
#define upk_alert(...) _upk_alert(_UPK_DIAG_LOCATION, __VA_ARGS__)
#define upk_crit(...) _upk_crit(_UPK_DIAG_LOCATION, __VA_ARGS__)
#define upk_error(...) _upk_error(_UPK_DIAG_LOCATION, __VA_ARGS__)
#define upk_warn(...) _upk_warn(_UPK_DIAG_LOCATION, __VA_ARGS__)
#define upk_notice(...) _upk_notice(_UPK_DIAG_LOCATION, __VA_ARGS__)
#define upk_info(...) _upk_info(_UPK_DIAG_LOCATION, __VA_ARGS__)
#define upk_verbose(...) _upk_verbose(_UPK_DIAG_LOCATION, __VA_ARGS__)
#define upk_debug0(...) _upk_debug0(_UPK_DIAG_LOCATION, __VA_ARGS__)
#define upk_debug1(...) _upk_debug1(_UPK_DIAG_LOCATION, __VA_ARGS__)

/**
  @}
  */

#endif