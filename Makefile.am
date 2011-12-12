SUBDIRS = libupkeeper controller clients 

ACLOCAL_AMFLAGS    = -I m4
#AM_CPPFLAGS_G = -DCONF_SYSCONFDIR=@conf_sysconfdir@ -DCONF_LOCALSTATEDIR=@conf_localstatedir@ -DCONF_LIBEXECDIR=@conf_libexecdir@

# ensure upkeeper.d is created, and provide an example configuration file.
nobase_dist_sysconf_DATA = upkeeper.d/README

docs: libupkeeper clients controller compat mainpage.doc libupkeeper/upkeeper
	doxygen

