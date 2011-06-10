AM_CPPFLAGS = -I$(top_builddir)/libupkeeper
LDADD = $(top_builddir)/libupkeeper/libupkeeper.la

bin_PROGRAMS = controller
controller_SOURCES = controller.c controller.h ctrl_protocol.c ctrl_buddy.c eventloop.c 
controller_DEPENDENCIES = $(top_builddir)/libupkeeper/libupkeeper.la
