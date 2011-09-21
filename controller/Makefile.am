
sbin_PROGRAMS = upk_controller 
upk_controller_SOURCES = controller.c controller_main.c controller.h ctrl_protocol.c ctrl_buddy.c ctrl_eventloop.c 
upk_controller_LDADD = $(top_builddir)/libupkeeper/libupkeeper.la 

#### upk_controller_LIBS = -ljson -lsqlite3 -lupkeeper

AM_CPPFLAGS = -I$(top_builddir)/libupkeeper
AM_LDFLAGS = -static
CFLAGS = 
LDFLAGS = 
LIBS = 
LDADD =
LD_RUN_PATH =

libexec_PROGRAMS = upk_buddy
upk_buddy_CFLAGS = -DUPK_STATIC_ERR_DECL 
upk_buddy_LDADD = $(top_builddir)/libupkeeper/libupkeeper_util.la
upk_buddy_SOURCES = buddy.c buddy_main.c $(top_builddir)/libupkeeper/upkeeper/upk_error.c $(top_builddir)/libupkeeper/upkeeper/error.h 


