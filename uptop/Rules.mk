include 	begin.mk

$(OBJS)	:= $(d)/main.o
$(DEPS)	:= $(OBJS_$(d):.o=.d)
$(BIN)	:= $(d)/uptop

CF_$(d) += -I$(d) -Istore
LL_$(d) := -lncurses store/store.a deps/sqlite/sqlite3.a

CLEAN	+= $(OBJS_$(d)) $(DEPS_$(d))
CHECK	+= $(d)/uptop.tap

$(d)/uptop.tap: $(d)/uptop
$(d)/uptop: $(d)/main.c $(LL_$(d))
	$(COMPLINK)

# Standard things
include 	end.mk
