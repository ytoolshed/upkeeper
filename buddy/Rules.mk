include 	begin.mk

$(OBJS)	:= $(d)/upk_buddy.o
$(DEPS)	:= $(OBJS_$(d):.o=.d)
$(BIN)	:= $(d)/buddytest $(d)/buddy
$(LIB)	:= $(d)/buddy.a 

CF_$(d) += -I$(d) -Istore
LL_$(d) := store/store.a $(d)/buddy.a deps/sqlite/sqlite3.a common/common.a
CLEAN   += $(d)/buddy.usage.c

CHECK	+= $(d)/buddytest.tap

$(d)/buddy.a:   $($(OBJS))
	$(ARCH)
$(d)/buddytest.tap: $(d)/buddytest
$(d)/buddytest: $(d)/main.c $(LL_$(d))
	$(COMPLINK)

$(d)/buddy: $(d)/buddy.c $(LL_$(d)) 
	$(COMPLINK)

$(d)/buddy.c: $(d)/buddy.usage.c

$(d)/buddy.usage.c: $(d)/buddy.usage
	sed -e 's/^/"/' -e 's/$$/"/' < $< >> $@

# Standard things
include 	end.mk

