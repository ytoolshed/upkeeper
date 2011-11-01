
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

#ifndef _UPK_COMPAT_H
#define _UPK_COMPAT_H

#include "upkeeper_config.h"
#include <sys/types.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef HAVE_SRANDOM
#define srand(x)    srandom(x)
#define rand()      random()
#endif

#ifdef HAVE_ISXDIGIT
#define c_isxdigit isxdigit
#else
#define c_isxdigit(c) ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))
#endif

#ifdef HAVE_GETPROGNAME
# define upk_getprogname getprogname()
#elif defined HAVE_GETEXECNAME
# define upk_getprogname getexecname()
#elif defined HAVE___PROGNAME
extern const char      *__progname;
# define upk_getprogname __progname
#endif

#ifndef HAVE_STRNLEN
extern size_t           strnlen(const char *restrict string, size_t maxlen);
#endif


#endif
