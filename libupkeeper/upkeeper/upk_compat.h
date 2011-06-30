#ifndef _UPK_COMPAT_H
#define _UPK_COMPAT_H

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

#endif
