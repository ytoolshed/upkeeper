include 	begin.mk

$(OBJS)	:= $(d)/upk_api.o
$(DEPS)	:= $(OBJS_$(d):.o=.d)
$(BIN)	:= $(d)/upk $(d)/upktest
$(LIB)  := $(d)/upkapi.a

CF_$(d) += -I$(d) -Istore
LL_$(d) := common/common.a store/store.a controller/controller.a buddy/buddy.a deps/sqlite/sqlite3.a 

CLEAN	+= $(OBJS_$(d)) $(DEPS_$(d))

CHECK	+= $(d)/upkapi.tap

$(d)/upkapi.a: $($(OBJS))
	$(ARCH)

$(d)/upkapi.tap: $(d)/upktest

$(d)/upktest: $(d)/test.c $(LL_$(d))
	$(COMPLINK) $(LL_upkapi)

$(d)/upk: $(d)/upk.c $(LL_$(d))
	$(COMPLINK) $(LL_upkapi)

# Standard things
include 	end.mk
