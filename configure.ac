#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([upkeeper], [1.0], [ytoolshed@yahoo-inc.com])

AC_GNU_SOURCE
LT_INIT

AM_INIT_AUTOMAKE([-W -Wall -Werror foreign])

AC_CONFIG_SRCDIR([upkeeper_config.h.in])
AC_CONFIG_HEADERS([upkeeper_config.h])


ACLOCAL_AMFLAGS="-I m4"
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_RANLIB
AC_C_INLINE
AC_PROG_LIBTOOL
AC_PROG_CC_C99
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])

# Checks for libraries.
AX_CHECK_EXT_LIB([ncurses],[main],[],[AC_MSG_ERROR([ncurses is required])])
AX_CHECK_EXT_LIB([sqlite3],[sqlite3_open],[],[AC_MSG_ERROR([sqlite is required])])
AX_CHECK_EXT_LIB([json],[json_tokener_parse],[],[AC_MSG_ERROR([json-c is required])])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h OS.h arpa/inet.h sys/file.h ctype.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UID_T
AC_TYPE_MODE_T

# Checks for library functions.
AX_FUNC_POLL
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_STRNLEN
AC_CHECK_FUNCS([dup2 memset select socket strdup strerror memchr strtol strcasecmp atexit mkdir gettimeofday strtoul])
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK

AC_SUBST([conf_localstatedir], [$localstatedir])
AC_SUBST([conf_sysconfdir], [$sysconfdir])

# TODO: we should have an AC_SUBST for this
# AX_FUNC_POLL

AC_CONFIG_FILES([Makefile
                 libupkeeper/Makefile
                 libupkeeper/tests/Makefile
				 controller/Makefile
                 store/Makefile])

# Why have we included a project we don't support in our source tree?
# AC_CONFIG_SUBDIRS([deps/sqlite
#                   deps/sqlite-3.7.3
#                   ])

AC_OUTPUT
