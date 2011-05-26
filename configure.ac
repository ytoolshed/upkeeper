#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#

AC_PREREQ(2.59)
AC_INIT([upkeeper],[0.01])
#AC_CONFIG_SRCDIR([buddy/upk_buddy.h])
AC_CONFIG_SRCDIR([libupkeeper/upkeeper/include/upkeeper_config.h.in])
AC_CONFIG_HEADERS([libupkeeper/upkeeper/include/upkeeper_config.h])

AC_CONFIG_MACRO_DIR([libltdl])

AC_SUBST([LIBTOOL_DEPS])


AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_FILES([
    Makefile
    libupkeeper/Makefile
])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_LIBTOOL


# Checks for libraries.
AC_CHECK_LIB([sqlite3], [sqlite3_open])
AC_CHECK_LIB([ncurses], [initscr])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])
AC_CHECK_HEADERS([sys/file.h sys/type.h])
# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
#AC_TYPE_UINT64_T

# Checks for library functions.
AC_CHECK_FUNCS([strerror fchdir gettimeofday mkdir mkfifo select])


AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([curses.h])

# Check functions
AC_FUNC_FORK
AC_FUNC_GETGROUPS
AC_FUNC_MALLOC
AC_FUNC_STRNLEN

AC_DEFUN([AX_HAVE_POLL], [dnl
  AC_MSG_CHECKING([for poll(2)])
  AC_CACHE_VAL([ax_cv_have_poll], [dnl
    AC_LINK_IFELSE([dnl
      AC_LANG_PROGRAM(
        [#include <poll.h>],
        [int rc; rc = poll((struct pollfd *)(0), 0, 0);])],
      [ax_cv_have_poll=yes],
      [ax_cv_have_poll=no])])
  AS_IF([test "${ax_cv_have_poll}" = "yes"],
    [AC_MSG_RESULT([yes])
$1],[AC_MSG_RESULT([no])
$2])
])dnl


AX_HAVE_POLL([AC_DEFINE([HAVE_POLL],[],[System includes poll(2)])],[])

AC_OUTPUT


