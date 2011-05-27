AUTOMAKE_OPTIONS = color-tests
AM_CPPFLAGS = @CHECK_CFLAGS@ -I$(top_builddir)/libupkeeper
LDADD = @CHECK_LIBS@ $(top_builddir)/libupkeeper/libupkeeper.la

check_PROGRAMS = v0_protocol_helpers v0_protocol_serializers upk_protocol_helpers upk_protocol_serializers

v0_protocol_helpers_DEPENDENCIES = v0_protocol_generic_value_checks.inc.c
v0_protocol_serializers_DEPENDENCIES = v0_protocol_generic_value_checks.inc.c
upk_protocol_helpers_DEPENDENCIES = v0_protocol_generic_value_checks.inc.c
upk_protocol_serializers_DEPENDENCIES = v0_protocol_generic_value_checks.inc.c

TESTS = v0_protocol_helpers \
		v0_protocol_serializers \
		upk_protocol_helpers \
		upk_protocol_serializers

valgrind: $(TESTS)
	for n in $(TESTS); do \
		CK_FORK=no \
		LD_LIBRARY_PATH="$(top_builddir)/libupkeeper/.libs/" \
		valgrind --tool=memcheck --leak-check=full --leak-resolution=high --track-origins=yes --show-reachable=yes $(builddir)/.libs/$$n; \
	done

