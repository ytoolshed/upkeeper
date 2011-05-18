UPK_LIBRARY_VERSION = 1:0:0
ACLOCAL_AMFLAGS     = -I libltdl
lib_LTLIBRARIES     = libupkeeper.la
libupkeeper_la_SOURCES = upkeeper/upk_protocol.c upkeeper/upk_v0_protocol.c upkeeper/upk_client.c upkeeper/controller.c \
						 upkeeper/upk_protocol_serializer.c upkeeper/upk_v0_protocol_serializer.c
libupkeeper_la_includedir=$(includedir)/upk-1.0/
libupkeeper_la_include_HEADERS = upkeeper/protocol.h upkeeper/v0_protocol.h upkeeper/types.h upkeeper/std_include.h \
								 upkeeper/error.h upkeeper/crc32.h
libupkeeper_la_libincludedir = $(libdir)/upk-1.0/include/
nodist_libupkeeper_la_libinclude_HEADERS = upkeeper/include/upkeeper_config.h
libupkeeper_la_LDFLAGS = -version-info $(UPK_LIBRARY_VERSION)
