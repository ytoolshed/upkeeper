AM_CPPFLAGS_G := $(CPPFLAGS) -I$(top_builddir)/libupkeeper
AM_LDFLAGS_G  := $(LDFLAGS)

AM_CPPFLAGS = @JSON_CPPFLAGS@ $(AM_CPPFLAGS_G)
AM_LDFLAGS  = @JSON_LDFLAGS@ @JSON_LIBS@ $(AM_LDFLAGS_G)

LIBS=
LDADD = $(top_builddir)/libupkeeper/libupkeeper.la

bin_PROGRAMS = upk upk_start upk_stop upk_reload
#upk_config upk_top upk_ps upk_ctl
