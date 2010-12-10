include 	begin.mk

$(OBJS)	:= $(d)/upk_buddy.o $(d)/buddy.o
$(DEPS)	:= $(OBJS_$(d):.o=.d)
$(BIN)	:= $(d)/buddytest $(d)/buddy
$(LIB)	:= $(d)/buddy.a 

CF_$(d) += -I$(d)
LL_$(d) := $(d)/buddy.a common/common.a store/store.a deps/sqlite/sqlite3.a

CHECK	+= $(d)/buddytest.tap
CLEAN   += $(d)/bt/buddy $(d)/bt/store.sqlite

$(d)/buddy.a:   $($(OBJS))
	$(ARCH)

$(d)/buddytest.tap: $(d)/buddytest 
$(d)/buddytest: $(d)/buddytest.c $(LL_$(d)) $(d)/buddy  
	$(COMPLINK) $(LL_buddy)

$(d)/buddy: $(d)/buddy.c common/common.a
	$(COMPLINK) common/common.a

# Standard things
include 	end.mk

