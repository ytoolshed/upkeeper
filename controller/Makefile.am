AM_CPPFLAGS = -I$(top_builddir)/libupkeeper
LIBS=
LDADD = $(top_builddir)/libupkeeper/libupkeeper.la 

sbin_PROGRAMS = upk_controller 
libexec_PROGRAMS = upk_buddy
upk_buddy_LDADD =
upk_buddy_CFLAGS = -DUPK_STATIC_ERR_DECL
upk_buddy_SOURCES = buddy.c buddy_main.c $(top_builddir)/libupkeeper/upkeeper/upk_error.c $(top_builddir)/libupkeeper/upkeeper/error.h

upk_controller_SOURCES = controller.c controller_main.c controller.h ctrl_protocol.c ctrl_buddy.c ctrl_eventloop.c 
upk_controller_DEPENDENCIES = $(top_builddir)/libupkeeper/libupkeeper.la
upk_controller_LDFLAGS = -ljson -lsqlite3

