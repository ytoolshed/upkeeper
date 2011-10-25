AM_CPPFLAGS_G := $(CPPFLAGS) -I$(top_builddir)/libupkeeper
#AM_LDFLAGS = -static
AM_LDFLAGS_G  := $(LDFLAGS)
#AM_LIBADD_G   := $(LIBADD)

AM_CPPFLAGS = @JSON_CPPFLAGS@ $(AM_CPPFLAGS_G)
AM_LDFLAGS  = @JSON_LDFLAGS@  $(AM_LDFLAGS_G)
#AM_LIBADD   = @JSON_LIBS@     $(AM_LIBADD_G)

LIBS=
LDADD = $(top_builddir)/libupkeeper/libupkeeper.la

bin_PROGRAMS = upk upk_start upk_stop upk_reload
#upk_LDFLAGS = -ljson
