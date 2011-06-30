AM_CPPFLAGS = -I$(top_builddir)/libupkeeper
LIBS=
LDADD = $(top_builddir)/libupkeeper/libupkeeper.la 

bin_PROGRAMS = controller buddy
buddy_LDADD =
buddy_CFLAGS = -DUPK_STATIC_ERR_DECL
buddy_SOURCES = buddy.c buddy_main.c $(top_builddir)/libupkeeper/upkeeper/upk_error.c $(top_builddir)/libupkeeper/upkeeper/error.h

controller_SOURCES = controller.c controller_main.c controller.h ctrl_protocol.c ctrl_buddy.c ctrl_eventloop.c 
controller_DEPENDENCIES = $(top_builddir)/libupkeeper/libupkeeper.la
controller_LDFLAGS = -ljson -lsqlite3

