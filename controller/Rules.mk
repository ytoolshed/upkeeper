include 	begin.mk

$(OBJS)	:= $(d)/main.o $(d)/upk.o
$(DEPS)	:= $(OBJS_$(d):.o=.d)
$(BIN)	:= $(d)/buddy-controller $(d)/upk

CF_$(d) += -I$(d) -Istore
LL_$(d) := store/store.a buddy/buddy.a deps/sqlite/sqlite3.a common/common.a

CLEAN	+= $(OBJS_$(d)) $(DEPS_$(d))

CHECK	+= $(d)/buddy-controller.t

$(d)/buddy-controller.tap: $(d)/buddy-controller

$(d)/buddy-controller: $(d)/main.c $(LL_$(d))
	$(COMPLINK) $(LL_controller)

$(d)/upk: $(d)/upk.c $(LL_$(d)) $(d)/buddy
	$(COMPLINK) $(LL_controller)

$(d)/buddy: buddy/buddy
	ln -sf ../$< $@

# Standard things
include 	end.mk

