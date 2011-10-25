
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


#include "upk_error.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef UPK_STATIC_ERR_DECL
#undef UPK_STATIC_ERR_DECL
#define UPK_STATIC_ERR_DECL static
#endif

#ifndef UPK_STATIC_ERR_DECL
#define UPK_STATIC_ERR_DECL
#endif

/* upk_error.c */
// static void _upk_default_error_rpt(upk_errno_t err);
// static void _upk_default_error_rpt_msg(upk_errno_t err, const char *msg);
static void             _upk_default_diag_output(upk_diaglvl_t diaglvl, const char *label, const char *loc,
                                                 const char *fmt, va_list ap);
static int32_t          _upk_diagnostic(upk_diaglvl_t diaglvl, const char *loc, const char *fmt, va_list ap);


int32_t                 upk_diag_verbosity = UPK_DIAGLVL_NOTICE;


/* *******************************************************************************************************************
   ****************************************************************************************************************** */
// __UPK_ERRORS_DESC_ARRAY;
// __UPK_DIAG_LABELS;

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
// err_rpt_callback_t __upk_error_rpt_callback = _upk_default_error_rpt;
// err_rpt_msg_callback_t __upk_error_rpt_msg_callback = _upk_default_error_rpt_msg;
diag_output_callback_t  __upk_diag_output_callback = _upk_default_diag_output;

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
const char             *
upk_strerror(upk_errno_t err)
{
    if(err < sizeof(__upk_errors))
        return __upk_errors[err];
    else
        return __upk_errors[0];
}


/* *******************************************************************************************************************
   ****************************************************************************************************************** */
/* 
   static void _upk_default_error_rpt(upk_errno_t err) { fprintf(stderr, "Error: `%s'\n", upk_strerror(err)); } */

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
/* 
   static void _upk_default_error_rpt_msg(upk_errno_t err, const char *msg) { fprintf(stderr, "Error: `%s': %s\n",
   upk_strerror(err), msg); } */

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
/* 
   void upk_report_error(upk_errno_t err) { if(__upk_error_rpt_callback) __upk_error_rpt_callback(err); } */

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
/* 
   void upk_report_error_msg(upk_errno_t err, const char *msg) { if(__upk_error_rpt_msg_callback)
   __upk_error_rpt_msg_callback(err, msg); } */

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
/* 
   err_rpt_callback_t upk_reg_error_callback(err_rpt_callback_t func) { err_rpt_callback_t oldfunc =
   __upk_error_rpt_callback;

   __upk_error_rpt_callback = func; return oldfunc; } */

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
/* 
   err_rpt_msg_callback_t upk_reg_err_msg_callback(err_rpt_msg_callback_t func) { err_rpt_msg_callback_t oldfunc =
   __upk_error_rpt_msg_callback;

   __upk_error_rpt_msg_callback = func; return oldfunc; } */

/* *******************************************************************************************************************
   ****************************************************************************************************************** */
diag_output_callback_t
upk_reg_diag_callback(diag_output_callback_t func)
{
    diag_output_callback_t  oldfunc = __upk_diag_output_callback;

    __upk_diag_output_callback = func;
    return oldfunc;
}


/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static void
_upk_default_diag_output(upk_diaglvl_t diaglvl, const char *label, const char *loc, const char *fmt, va_list ap)
{
    UPK_STATIC_ERR_DECL bool show_label, show_loc;

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


/* ********************************************************************************************************************
   ******************************************************************************************************************* */
static                  int32_t
_upk_diagnostic(upk_diaglvl_t diaglvl, const char *loc, const char *fmt, va_list ap)
{
    if(!__upk_diag_output_callback)
        return 0;

    switch (diaglvl) {
    case UPK_DIAGLVL_FATAL:
        __upk_diag_output_callback(diaglvl, upk_diag_label_idx[diaglvl], loc, fmt, ap);
        exit(1);
        break;
    default:
        if(upk_diag_verbosity >= diaglvl) {
            __upk_diag_output_callback(diaglvl, upk_diag_label_idx[diaglvl], loc, fmt, ap);
        }
        break;
    }
    return 0;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
int32_t
_upk_fatal(const char *loc, const char *fmt, ...)
{
    UPK_STATIC_ERR_DECL va_list ap;
    UPK_STATIC_ERR_DECL int32_t ret = 0;

    va_start(ap, fmt);
    ret = _upk_diagnostic(UPK_DIAGLVL_FATAL, loc, fmt, ap);
    va_end(ap);
    return ret;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
int32_t
_upk_alert(const char *loc, const char *fmt, ...)
{
    UPK_STATIC_ERR_DECL va_list ap;
    UPK_STATIC_ERR_DECL int32_t ret = 0;

    va_start(ap, fmt);
    ret = _upk_diagnostic(UPK_DIAGLVL_ALERT, loc, fmt, ap);
    va_end(ap);
    return ret;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
int32_t
_upk_crit(const char *loc, const char *fmt, ...)
{
    UPK_STATIC_ERR_DECL va_list ap;
    UPK_STATIC_ERR_DECL int32_t ret = 0;

    va_start(ap, fmt);
    ret = _upk_diagnostic(UPK_DIAGLVL_CRIT, loc, fmt, ap);
    va_end(ap);
    return ret;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
int32_t
_upk_error(const char *loc, const char *fmt, ...)
{
    UPK_STATIC_ERR_DECL va_list ap;
    UPK_STATIC_ERR_DECL int32_t ret = 0;

    va_start(ap, fmt);
    ret = _upk_diagnostic(UPK_DIAGLVL_ERROR, loc, fmt, ap);
    va_end(ap);
    return ret;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
int32_t
_upk_warn(const char *loc, const char *fmt, ...)
{
    UPK_STATIC_ERR_DECL va_list ap;
    UPK_STATIC_ERR_DECL int32_t ret = 0;

    va_start(ap, fmt);
    ret = _upk_diagnostic(UPK_DIAGLVL_WARN, loc, fmt, ap);
    va_end(ap);
    return ret;
}

/* ********************************************************************************************************************
   ******************************************************************************************************************* */
int32_t
_upk_notice(const char *loc, const char *fmt, ...)
{
    UPK_STATIC_ERR_DECL va_list ap;
    UPK_STATIC_ERR_DECL int32_t ret = 0;

    va_start(ap, fmt);
    ret = _upk_diagnostic(UPK_DIAGLVL_NOTICE, loc, fmt, ap);
    va_end(ap);
    return ret;
}



/* ********************************************************************************************************************
   ******************************************************************************************************************* */
int32_t
_upk_info(const char *loc, const char *fmt, ...)
{
    UPK_STATIC_ERR_DECL va_list ap;
    UPK_STATIC_ERR_DECL int32_t ret = 0;

    va_start(ap, fmt);
    ret = _upk_diagnostic(UPK_DIAGLVL_INFO, loc, fmt, ap);
    va_end(ap);
    return ret;
}


/* ********************************************************************************************************************
   ******************************************************************************************************************* */
int32_t
_upk_verbose(const char *loc, const char *fmt, ...)
{
    UPK_STATIC_ERR_DECL va_list ap;
    UPK_STATIC_ERR_DECL int32_t ret = 0;

    va_start(ap, fmt);
    ret = _upk_diagnostic(UPK_DIAGLVL_VERBOSE, loc, fmt, ap);
    va_end(ap);
    return ret;
}


/* ********************************************************************************************************************
   ******************************************************************************************************************* */
int32_t
_upk_debug0(const char *loc, const char *fmt, ...)
{
    UPK_STATIC_ERR_DECL va_list ap;
    UPK_STATIC_ERR_DECL int32_t ret = 0;

    va_start(ap, fmt);
    ret = _upk_diagnostic(UPK_DIAGLVL_DEBUG0, loc, fmt, ap);
    va_end(ap);
    return ret;
}


/* ********************************************************************************************************************
   ******************************************************************************************************************* */
int32_t
_upk_debug1(const char *loc, const char *fmt, ...)
{
    UPK_STATIC_ERR_DECL va_list ap;
    UPK_STATIC_ERR_DECL int32_t ret = 0;

    va_start(ap, fmt);
    ret = _upk_diagnostic(UPK_DIAGLVL_DEBUG1, loc, fmt, ap);
    va_end(ap);
    return ret;
}
