AC_DEFUN([_AX_WITH_EXT_LIB],
         [AS_REQUIRE([AS_HELP_STRING])
		  AC_REQUIRE([AC_ARG_WITH])
		  AC_ARG_WITH([$1],
			          [AS_HELP_STRING([--with-$1=PATH],[specify path to $1 library installation @<:@default=auto@:>@])],
					  [ax_with_$1=$withval], [ax_with_$1=auto])
         ])

AC_DEFUN([_AX_CHECK_EXT_LIB],
         [AS_REQUIRE([AS_IF])
		  AS_REQUIRE([AS_CASE])
		  AS_IF([test "x$ax_with_$1" != "xauto"],
				[oldLDFLAGS="$LDFLAGS"
				 oldCPPFLAGS="$CPPFLAGS"
				 AS_CASE(["$ax_with_$1"],[./*|../*],[$ax_with_$1="`pwd`/$withval"])
				 AS_IF([test -d "${ax_with_$1}/lib"], [LDFLAGS="-L${ax_with_$1}/lib $LDFLAGS"])
				 AS_IF([test -d "${ax_with_$1}/include"], [CPPFLAGS="-I${ax_with_$1}/include $CPPFLAGS"])
	            ])
         ])
				 
AC_DEFUN([AX_CHECK_EXT_LIB],
		 [AC_REQUIRE([AC_CHECK_LIB])
	 	  AC_REQUIRE([_AX_WITH_EXT_LIB])
		  AC_REQUIRE([_AX_CHECK_EXT_LIB])
		  _AX_WITH_EXT_LIB([$1])
		  _AX_CHECK_EXT_LIB([$1])
		  AC_CHECK_LIB([$1],[$2],[$3],[$4])
		 ])


