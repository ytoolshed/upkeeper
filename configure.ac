#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63)
AC_INIT([upkeeper],[1.0],[ytoolshed@yahoo-inc.com])

AC_USE_SYSTEM_EXTENSIONS
LT_INIT

AM_INIT_AUTOMAKE([-W -Wall -Werror foreign])
AM_PROG_CC_C_O

AC_CONFIG_SRCDIR([upkeeper_config.h.in])
AC_CONFIG_HEADERS([upkeeper_config.h])

ACLOCAL_AMFLAGS="-I m4"
AC_CONFIG_MACRO_DIR([m4])

PKG_PROG_PKG_CONFIG

# Checks for programs.
# AC_PROG_RANLIB
AC_C_INLINE
AC_C_RESTRICT
LT_INIT
AC_PROG_CC_C99
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET


# Checks for libraries.
#PKG_CHECK_MODULES([CHECK], [check >= 0.9.4]) dnl [AC_MSG_WARN([The "check" C unit testing library was not found! you will not be able to run "make check"])])
AX_CHECK_EXT_LIB([ncurses],[main],[],[AC_MSG_ERROR([ncurses is required])])
AX_CHECK_EXT_LIB([sqlite3],[sqlite3_open],[],[AC_MSG_ERROR([sqlite is required])])
AX_CHECK_EXT_LIB([json],[json_tokener_parse],[],[AC_MSG_ERROR([json-c is required])])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h OS.h arpa/inet.h sys/file.h ctype.h float.h stddef.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_INT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UID_T
AC_TYPE_MODE_T

# Checks for library functions.
# AX_FUNC_POLL
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_STRNLEN
AC_FUNC_REALLOC
AC_CHECK_FUNCS([dup2 memset select socket strdup strerror memchr strtol strcasecmp atexit mkdir gettimeofday strtoul strncasecmp strchr strstr strnlen])
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK

## Check whether or not symbol '__progname' is defined
AX_CHECK___PROGNAME

#if test "x$prefix" = "xNONE"; then
#	prefix="/usr/local"
#fi

#if test "x$exec_prefix" = "xNONE"; then 
#	exec_prefix="$prefix"
#fi

#conf_libexecdir="`eval echo \"$libexecdir\"`"
#conf_localstatedir="`eval echo \"$localstatedir\"`"
#conf_sysconfdir="`eval echo \"$sysconfdir\"`"

#AC_DEFINE_UNQUOTED([CONF_LIBEXECDIR], $conf_libexecdir, [Define to the value of libexecdir])
#AC_DEFINE_UNQUOTED([CONF_LOCALSTATEDIR], $conf_localstatedir, [Define to the value of localstatedir])
#AC_DEFINE_UNQUOTED([CONF_SYSCONFDIR], $conf_sysconfdir, [Define to the value of sysconfdir])

AX_ACDEFINE_INSTDIRS

#AC_SUBST([conf_libexecdir], [$libexecdir])
#AC_SUBST([conf_localstatedir], [$localstatedir])
#AC_SUBST([conf_sysconfdir], [$sysconfdir])
##AX_ACSUBST_INSTDIRS
#AX_SHELL_EXPANDVAR([libexecdir])

# TODO: we should have an AC_SUBST for this
# AX_FUNC_POLL

AC_CONFIG_FILES([Makefile
                 libupkeeper/Makefile
                 libupkeeper/tests/Makefile
				 controller/Makefile
				 clients/Makefile
                 ])

# build this seperately, and pass to configure with '--with-sqlite=/path/to/destdir'
# AC_CONFIG_SUBDIRS([deps/sqlite
#                   deps/sqlite-3.7.3
#                   ])

AC_OUTPUT
