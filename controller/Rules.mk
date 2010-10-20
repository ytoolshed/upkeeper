include 	begin.mk

$(OBJS)	:= $(d)/main.o
$(DEPS)	:= $(OBJS_$(d):.o=.d)
$(BIN)	:= $(d)/buddy-controller

CF_$(d) += -I$(d) -Istore
LL_$(d) := store/store.a $(d)/buddy.a deps/sqlite/sqlite3.a common/common.a

CLEAN	+= $(OBJS_$(d)) $(DEPS_$(d))
CHECK	+= $(d)/buddytest.tap

$(d)/buddy.a:   $($(OBJS))
	$(ARCH)
$(d)/buddy-controller.tap: $(d)/buddy-controller
$(d)/buddy-controller: $(d)/main.c $(LL_$(d))
	$(COMPLINK)

# Standard things
include 	end.mk

