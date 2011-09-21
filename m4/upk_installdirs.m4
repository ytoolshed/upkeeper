
# serial 20110711001

AC_DEFUN([AX_SHELL_EXPANDVAR],
	[m4_bpatsubst([AS_ECHO_N(["$$1"])],["[\"\'\$]"],[])
])

AC_DEFUN([AX_LOOP_INSTDIRS],[m4_map([$1], [prefix, execprefix, bindir, sbindir, libexecdir, sysconfdir, sharedstatedir, localstatedir, libdir, includedir,dnl
oldincludedir, datarootdir, datadir, infodir, localedir, mandir, docdir, htmldir, dvidir, pdfdir, psdir])dnl
])

AC_DEFUN([AX_INSTDIR_VARNAME],[CONF_]m4_toupper([$1]))

AC_DEFUN([AX_SUBST_INSTDIR],
	[m4_apply([AC_SUBST],[AX_INSTDIR_VARNAME([$1])],["AX_SHELL_EXPANDVAR([$1])"])dnl
])

AC_DEFUN([AX_ACSUBST_INSTDIRS],
	[AX_REQUIRE([AX_LOOP_INSTDIRS])
	 AX_LOOP_INSTDIRS([AX_SUBST_INSTDIR])]
)

AC_DEFUN([AX_GEN_INSTDIR_DEFINE],
	[m4_append([AX_INSTDIRS_CPPFLAGS],[-D]AX_INSTDIR_VARNAME([$1])[=@]AX_INSTDIR_VARNAME([$1])[@],[ ])]
)

AC_DEFUN([AX_DEFINE_INSTDIRS],
	[AX_REQUIRE([AX_LOOP_INSTDIRS]) AX_LOOP_INSTDIRS([AX_GEN_INSTDIR_DEFINE])]
)
	

	








