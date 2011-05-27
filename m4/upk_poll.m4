AC_DEFUN([AX_HAVE_POLL], [AC_REQUIRE([_AX_HAVE_POLL])])

AC_DEFUN([_AX_HAVE_POLL], [dnl
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

AC_DEFUN([AX_FUNC_POLL],[AX_HAVE_POLL([AC_DEFINE([HAVE_POLL],[],[System includes poll(2)])],[]) ])


# should specify a AC_SUBST macro to hotfix for a missing poll(2) if we
