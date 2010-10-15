#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([upkeeper], [0.01])
AC_CONFIG_SRCDIR([buddy/upk_buddy.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# The Mac's gcc wants this option to handle a external global int
# in the shared library (the variable DEBUG) which is defined in
# the main program
# AX_CFLAGS_GCC_OPTION([undefined dynamic_lookup])
AC_ARG_ENABLE(shared, [AS_HELP_STRING(
  [--enable-shared], [build shared libraries [default=yes]])], 
  [], [enable_shared=yes])
if test x"$enable_threadsafe" != "xno"; then
  THREADSAFE_FLAGS=-DSQLITE_THREADSAFE=1
  AC_SEARCH_LIBS(pthread_create, pthread)
fi
AC_SUBST(THREADSAFE_FLAGS)

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lsqlite3':
AC_CHECK_LIB([sqlite3], [sqlite3_open])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_CHECK_FUNCS([strerror])

AC_CONFIG_FILES([Makefile
		 buddy/Makefile
                 controller/Makefile
                 store/Makefile])
AC_OUTPUT
