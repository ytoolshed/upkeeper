SUBDIRS = tests

UPK_LIBRARY_VERSION = 1:0:0
CPPFLAGS = -DCONF_SYSCONFDIR=@conf_sysconfdir@ -DCONF_LOCALSTATEDIR=@conf_localstatedir@ -DCONF_LIBEXECDIR=@conf_libexecdir@

lib_LTLIBRARIES     = libupkeeper.la
noinst_LTLIBRARIES = libupkeeper_proto.la libupkeeper_util.la libupkeeper_error.la

bin_PROGRAMS = t json
t_LDADD = libupkeeper.la
json_LDADD = libupkeeper.la

libupkeeper_util_la_SOURCES = upkeeper/upk_crc32.c upkeeper/uuid.c upkeeper/upk_json.c upkeeper/upk_config.c upkeeper/upk_util.c
libupkeeper_error_la_SOURCES = upkeeper/upk_error.c
libupkeeper_util_la_EXTRA = upkeeper/strnlen.c upkeeper/stdbool.in.h
libupkeeper_proto_la_SOURCES = upkeeper/upk_protocol.c upkeeper/upk_v0_protocol_serializer.c upkeeper/upk_protocol_serializer.c upkeeper/upk_v0_protocol.c

libupkeeper_la_SOURCES = 
libupkeeper_la_LIBADD = libupkeeper_util.la libupkeeper_proto.la libupkeeper_error.la
                                                 
libupkeeper_la_includedir=$(includedir)
nobase_libupkeeper_la_include_HEADERS = upkeeper/upk_protocol.h upkeeper/upk_v0_protocol.h upkeeper/upk_types.h upkeeper/upk_std_include.h \
								 upkeeper/upk_error.h upkeeper/upk_crc32.h upkeeper/upk_uuid.h upkeeper/upk_json.h upkeeper/upk_config.h
libupkeeper_la_include_HEADERS = upkeeper.h

libupkeeper_la_libincludedir = $(libdir)/upkeeper-1.0/include/
nodist_libupkeeper_la_libinclude_HEADERS = $(top_builddir)/upkeeper_config.h
libupkeeper_la_LDFLAGS = -version-info $(UPK_LIBRARY_VERSION)

valgrind: 
	(cd tests; make valgrind)
