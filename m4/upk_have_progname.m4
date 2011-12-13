AC_DEFUN([_AX_CHECK___PROGNAME],
		 [AC_REQUIRE([AC_TRY_LINK])
          AC_TRY_LINK([], [extern const char *__progname; const char *foo; foo = __progname;], [AC_DEFINE([HAVE___PROGNAME], 1, [Define if libc defines __progname])])
		  ])

AC_DEFUN([AX_CHECK___PROGNAME],
		 [AC_REQUIRE([AC_TRY_LINK])
		  AC_REQUIRE([_AX_CHECK___PROGNAME])
		  _AX_CHECK___PROGNAME
         ])
		  
		
