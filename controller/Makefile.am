AM_CPPFLAGS_G := $(CPPFLAGS) -I$(top_builddir)/libupkeeper
AM_LDFLAGS_G  := $(LDFLAGS)

AM_CPPFLAGS = @SQLITE3_CPPFLAGS@ @JSON_CPPFLAGS@ $(AM_CPPFLAGS_G)
AM_LDFLAGS  = @SQLITE3_LDFLAGS@ @SQLITE3_LIBS@ @JSON_LDFLAGS@ @JSON_LIBS@ $(AM_LDFLAGS_G)

ctrl_sql.h: *.sql $(top_srcdir)/scripts/create_sql_header.sh
	chmod 755 $(top_srcdir)/scripts/create_sql_header.sh; \
	$(top_srcdir)/scripts/create_sql_header.sh "$(top_builddir)"


BUILT_SOURCES = ctrl_sql.h
sbin_PROGRAMS = upk_controller 
upk_controller_SOURCES = controller.c controller_main.c controller.h ctrl_protocol.c ctrl_buddy.c ctrl_eventloop.c ctrl_data.c ctrl_data.h $(top_builddir)/upkeeper_config.h 
nodist_upk_controller_SOURCES = ctrl_sql.h
upk_controller_LDADD = $(top_builddir)/libupkeeper/libupkeeper.la 

libexec_PROGRAMS = upk_buddy
upk_buddy_CFLAGS = -DUPK_STATIC_ERR_DECL 
upk_buddy_LDFLAGS = -static
upk_buddy_LDADD = $(top_builddir)/libupkeeper/libupkeeper_util.la
upk_buddy_SOURCES = buddy.c buddy_main.c $(top_builddir)/libupkeeper/upkeeper/upk_error.c $(top_builddir)/libupkeeper/upkeeper/error.h $(top_builddir)/upkeeper_config.h


