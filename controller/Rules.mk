include 	begin.mk

$(OBJS)	:= $(d)/controller.o
$(DEPS)	:= $(OBJS_$(d):.o=.d)
$(BIN)	:= $(d)/buddy-controller $(d)/upk
$(LIB)  := $(d)/controller.a

CF_$(d) += -I$(d) -Istore
LL_$(d) := common/common.a store/store.a controller/controller.a buddy/buddy.a deps/sqlite/sqlite3.a 

CLEAN	+= $(OBJS_$(d)) $(DEPS_$(d))

CHECK	+= $(d)/buddy-controller.tap

$(d)/controller.a: $($(OBJS))
	$(ARCH)

$(d)/buddy-controller.tap: $(d)/buddy-controller

$(d)/buddy-controller: $(d)/main.c $(LL_$(d))
	$(COMPLINK) $(LL_controller)

$(d)/upk: $(d)/upk.c $(LL_$(d)) $(d)/buddy
	$(COMPLINK) $(LL_controller)

$(d)/buddy: buddy/buddy
	ln -sf ../$< $@

# Standard things
include 	end.mk
