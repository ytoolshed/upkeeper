include 	begin.mk

$(OBJS)	:= $(d)/upk_buddy.o
$(DEPS)	:= $(OBJS_$(d):.o=.d)
$(BIN)	:= $(d)/buddytest
$(LIB)	:= $(d)/buddy.a 

CF_$(d) += -I$(d) -Istore
LL_$(d) := store/store.a $(d)/buddy.a deps/sqlite/sqlite3.a common/common.a

CHECK	+= $(d)/buddytest.tap

$(d)/buddy.a:   $($(OBJS))
	$(ARCH)
$(d)/buddytest.tap: $(d)/buddytest
$(d)/buddytest: $(d)/main.c $(LL_$(d))
	$(COMPLINK)

# Standard things
include 	end.mk

