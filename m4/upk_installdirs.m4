
# serial 20110711004
# Yes, I know the autoconf manual says you shouldn't do this, however, because these should only be used to establish default values, and you should 
# always be able to specify another path for a configuraton file, and specify in that configuration file alternate paths to the other things that use
# these macros, I'm ok with it in this instance. Do not abuse for anything that can not be overridden at runtime 

AC_DEFUN([AX_LOOP_INSTDIRS],[m4_map([$1], [prefix, exec_prefix, bindir, sbindir, libexecdir, sysconfdir, sharedstatedir, localstatedir, libdir, includedir,dnl
oldincludedir, datarootdir, datadir, infodir, localedir, mandir, docdir, htmldir, dvidir, pdfdir, psdir])dnl
])

AC_DEFUN([AX_INSTDIR_VARNAME],[CONF_]m4_toupper([$1]))

AC_DEFUN([AX_DEFINE_INSTDIR],
	[m4_define([_CONF_VAR],[conf_$1])
	_CONF_VAR="$$1"
	_CONF_VAR_COUNT=`echo "$_CONF_VAR" | $GREP -c '\\\$'` #'
	while test $_CONF_VAR_COUNT -gt 0; do 
		_CONF_VAR=`eval echo "$_CONF_VAR"` #''
		_CONF_VAR_COUNT=`echo "$_CONF_VAR" | $GREP -c '\\\$'` #'
	done
	AC_DEFINE_UNQUOTED(m4_toupper(_CONF_VAR),[$_CONF_VAR],define m4_toupper(_CONF_VAR) to $$1) # ''
])

AC_DEFUN([_AX_ACDEFINE_INSTDIRS],
	[AS_IF([test "x$prefix" = "xNONE"],[prefix="/usr/local"])dnl
	AS_IF([test "x$exec_prefix" = "xNONE"],[exec_prefix="$prefix"])dnl
	AX_LOOP_INSTDIRS([AX_DEFINE_INSTDIR])]
)

AC_DEFUN([AX_ACDEFINE_INSTDIRS],
	[AC_REQUIRE([_AX_ACDEFINE_INSTDIRS])
	 _AX_ACDEFINE_INSTDIRS
])

	








