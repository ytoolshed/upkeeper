AUTOMAKE_OPTIONS = subdir-objects 
SUBDIRS = tests
UPK_LIBRARY_VERSION = 1:0:0

AM_CPPFLAGS_G := $(CPPFLAGS)
AM_LDFLAGS_G  := $(LDFLAGS)
AM_LIBADD_G   := $(LIBADD)

AM_CPPFLAGS = @JSON_CPPFLAGS@ $(AM_CPPFLAGS_G)
AM_LDFLAGS  = @JSON_LDFLAGS@  $(AM_LDFLAGS_G)
AM_LIBADD   = @JSON_LIBS@     $(AM_LIBADD_G)

lib_LTLIBRARIES     = libupkeeper.la
noinst_LTLIBRARIES = libupkeeper_proto.la libupkeeper_util.la libupkeeper_client.la libupkeeper_core.la

libupkeeper_util_la_SOURCES = upkeeper/upk_crc32.c upkeeper/uuid.c  upkeeper/upk_util.c $(top_builddir)/upkeeper_config.h
#libupkeeper_util_la_CPPFLAGS = $(AM_CPPFLAGS)
#libupkeeper_util_la_LDFLAGS = $(AM_LDFLAGS)
libupkeeper_core_la_SOURCES = upkeeper/upk_config.c upkeeper/upk_json.c upkeeper/upk_error.c $(top_builddir)/upkeeper_config.h
libupkeeper_core_la_LIBADD = $(AM_LIBADD)
libupkeeper_proto_la_SOURCES = upkeeper/upk_protocol.c upkeeper/upk_v0_protocol_serializer.c \
							   upkeeper/upk_protocol_serializer.c upkeeper/upk_v0_protocol.c upkeeper/upk_network.c \
							   $(top_builddir)/upkeeper_config.h 
libupkeeper_client_la_SOURCES = upkeeper/upk_client.c upkeeper/upk_client_net.c $(top_builddir)/upkeeper_config.h 

libupkeeper_la_SOURCES =
libupkeeper_la_LDFLAGS = -version-info $(UPK_LIBRARY_VERSION)
libupkeeper_la_LIBADD = libupkeeper_util.la libupkeeper_proto.la libupkeeper_core.la libupkeeper_client.la
                                                 
libupkeeper_util_la_includedir=$(includedir)
nobase_libupkeeper_util_la_include_HEADERS = upkeeper/upk_crc32.h upkeeper/upk_uuid.h upkeeper/upk_json.h upkeeper/upk_util.h

libupkeeper_core_la_includedir=$(includedir)
nobase_libupkeeper_core_la_include_HEADERS = upkeeper/upk_json.h upkeeper/upk_config.h upkeeper/upk_error.h upkeeper/upk_types.h upkeeper/upk_std_include.h

libupkeeper_proto_la_includedir=$(includedir)
nobase_libupkeeper_proto_la_include_HEADERS = upkeeper/upk_protocol.h upkeeper/upk_v0_protocol.h upkeeper/upk_network.h

libupkeeper_client_la_includedir=$(includedir)
nobase_libupkeeper_client_la_include_HEADERS = upkeeper/upk_client_net.h

libupkeeper_la_includedir=$(includedir)
nobase_libupkeeper_la_include_HEADERS = 

libupkeeper_la_include_HEADERS = upkeeper.h

libupkeeper_la_libincludedir = $(libdir)/upkeeper-1.0/include/
nodist_libupkeeper_la_libinclude_HEADERS = $(top_builddir)/upkeeper_config.h

bin_PROGRAMS = t json s rmrf
t_LDADD = libupkeeper.la
rmrf_LDADD = libupkeeper.la
json_LDADD = libupkeeper.la
s_LDADD = libupkeeper.la


valgrind: 
	(cd tests; make valgrind)
