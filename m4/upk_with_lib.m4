

AC_DEFUN([AX_WITH_EXT_LIB], [dnl
    AS_REQUIRE([AS_HELP_STRING])
    AC_REQUIRE([AC_ARG_WITH])
    m4_ifval($1,[dnl
        AC_ARG_WITH([$1], [AS_HELP_STRING([--with-$1=PATH],[specify path to $1 library installation @<:@default=auto@:>@])],
                          [ax_with_$1=$withval], [ax_with_$1=auto])
    ])
])

AC_DEFUN([AX_SETUP_EXT_LIB],[dnl
    AS_REQUIRE([AS_IF])
    AS_REQUIRE([AS_CASE])
    AS_REQUIRE([AS_VAR_PUSHDEF])
    AS_REQUIRE([AS_VAR_POPDEF])
    AS_REQUIRE([AS_VAR_IF])

    m4_ifval($1, [dnl
        AS_VAR_PUSHDEF([ax_tmp_with],[ax_with_$1])
        AS_VAR_IF([ax_tmp_with],[auto],[:],[dnl
            AS_CASE(["$ax_tmp_with"],[./*|../*],[ax_tmp_with="`pwd`/$withval"]) #''
            AS_IF([test -d "${ax_tmp_with}/lib"],[dnl
                AS_VAR_SET([m4_toupper([$1_LDFLAGS])],["-L${ax_tmp_with}/lib"])
            ])
            AS_IF([test -d "${ax_tmp_with}/lib64"],[dnl
                AS_VAR_SET([m4_toupper([$1_LDFLAGS])],["m4_toupper([$$1_LDFLAGS]) -L${ax_tmp_with}/lib64"])
            ])
            AS_IF([test -d "${ax_tmp_with}/include"],[dnl
                AS_VAR_SET([m4_toupper([$1_CPPFLAGS])],["-I${ax_tmp_with}/include"])
            ])
        ])
        AS_VAR_POPDEF([ax_tmp_with])
    ])
])

m4_define([ax_store_val],[m4_define([$1],[$2])])

AC_DEFUN([ax_make_tmpvars],[dnl
    AS_REQUIRE([_AS_TR_SH_PREPARE]) dnl
    ax_store_val([ax_extlib_varname],[m4_toupper([[$1_$2]])]) dnl
    m4_if([ax_extlib_varname],[_],[:],[ dnl
        AS_VAR_SET_IF([ax_extlib_varname],[ dnl
            dnl AS_ECHO(["m4_bpatsubst(["$ax_extlib_varname"],["[\"\'\$]"],[])"])
            ax_store_val([variable_payload],[m4_unquote([AX_SH_ACTION([ax_extlib_varname])])]) dnl
            m4_bpatsubst([variable_payload],[\"],[]) dnl
            AC_MSG_CHECKING([$1 flags ax_extlib_varname])
            AC_MSG_RESULT(variable_payload)
            m4_apply([AC_SUBST],[ax_extlib_varname])
        ])
    ])
])

AC_DEFUN([AX_SET_EXTLIB],[dnl
    AS_REQUIRE([_AS_TR_SH_PREPARE])
    m4_ifval($1,[$1],[dnl
          AC_DEFINE(m4_toupper([HAVE_LIB$2]), 1, [Define to 1 if you have lib$2])
          AS_VAR_SET([m4_toupper([$2_LIBS])], [-l$2])
          m4_map([ax_make_tmpvars], [[[$2],[LIBS]],[[$2],[LDFLAGS]],[[$2],[CPPFLAGS]]])
    ])
])

                 
AC_DEFUN([AX_CHECK_EXT_LIB],
         [AC_REQUIRE([AC_CHECK_LIB])
          AC_REQUIRE([AX_WITH_EXT_LIB])
          AC_REQUIRE([AX_SETUP_EXT_LIB])
          AX_WITH_EXT_LIB([$1])
          AX_SETUP_EXT_LIB([$1])
          AS_VAR_SET([tmp_LDFLAGS],["$LDFLAGS"])
          AS_VAR_SET([LDFLAGS], ["$LDFLAGS m4_toupper([$$1_LDFLAGS])"])
          AC_CHECK_LIB([$1],[$2],[AX_SET_EXTLIB($3,$1)], [dnl
			AS_VAR_SET([m4_toupper([$1_LIBS])], [-l$1])
			m4_map([ax_make_tmpvars], [[[$2],[LIBS]]])
			$4
		  ])
          AS_VAR_SET([LDFLAGS],["$tmp_LDFLAGS"])
         ])

