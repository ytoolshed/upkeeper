# serial 20110726004

m4_ifndef([m4_ifblank],[dnl
	m4_defun_init([m4_ifblank], [m4_if(m4_translit([[$1]],[ ][	][ ]), [], [$2], [$3])])
])

m4_ifndef([m4_ifnblank],[dnl
	m4_defun_init([m4_ifnblank], [m4_if(m4_translit([[$1]],[ ][	][ ]), [], [$3], [$2])])
])

m4_ifndef([_AS_LITERAL_IF],[dnl
	AC_DEFUN([_AS_LITERAL_IF],[dnl
	m4_if(m4_index([$1], [@S|@]), [-1], [$0_(m4_translit([$1],
	[-:=%/@{}[]#(),.$2]]]m4_dquote(m4_dquote(m4_defn([m4_cr_symbols2])))[[,
	[++++++$$`````]))], [$0_NO])])

	AC_DEFUN([_AS_LITERAL_IF_],[dnl
		m4_if(m4_translit([$1], [+]), [], [$0YES], 
		m4_translit([$1], [$]), [], [m4_default], [$0NO]) dnl
	])

	AC_DEFUN([_AS_LITERAL_IF_YES], [$3])
	AC_DEFUN([_AS_LITERAL_IF_NO], [$2])
])
# '''''

m4_ifndef([AS_LITERAL_WORD_IF],[dnl
	AC_DEFUN([AS_LITERAL_WORD_IF],[dnl
		_AS_LITERAL_IF(m4_expand([$1]))([$4], [$3], [$2])
	])
])

AC_DEFUN([AX_VAR_GET],[dnl
	AS_LITERAL_IF([$1], [$$1],[dnl
	`eval 'as_val=${'_AS_ESCAPE([[$1]], [`], [\])'}; AS_ECHO(["$as_val"])'`])
]) #'

m4_ifndef([AS_VAR_COPY],[dnl
	AC_DEFUN([AS_VAR_COPY],[dnl
		AS_REQUIRE([AS_LITERAL_WORD_IF])dnl
		AS_LITERAL_WORD_IF([$1[]$2], [$1=$$2], [eval $1=\$$2])dnl
	])
])

AC_DEFUN([AX_SH_ACTION],[dnl
	AS_REQUIRE([AS_PREPARE])dnl
	AS_REQUIRE([_AS_TR_SH_PREPARE])dnl
	m4_divert_push([KILL])dnl
	m4_append([_AS_CLEANUP],[m4_divert_text([6])])dnl
	AS_LITERAL_IF([$1],["$$1"], [`eval 'echo "$$1"'`], [`eval 'echo "$$1"'`]) dnl
	m4_divert_pop()dnl
]) #''

AC_DEFUN([AX_DEPENDANT],[
	AS_REQUIRE([m4_ifnblank])
	AS_REQUIRE([m4_ifblank])
	AS_REQUIRE([AS_TR_SH])
	AC_REQUIRE([AX_VAR_GET])
	AC_REQUIRE([AS_VAR_GET])
	AC_REQUIRE([AS_VAR_COPY])
	AC_REQUIRE([AX_SH_ACTION])
	AS_REQUIRE([AS_LITERAL_WORD_IF])
	[$2],[$1]
])
