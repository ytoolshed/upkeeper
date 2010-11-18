# push directory on stack
include begin.mk

# objects, targets
$(DEPS)		:= $(OBJS_$(d):.o=.d)
$(BIN)		:= $(d)/sqlite-repair

LL_$(d) := store/store.a deps/sqlite/sqlite3.a

$(d)/sqlite-repair: $(d)/sqlite-repair.c store/store.a deps/sqlite/sqlite3.a
	$(COMPLINK)

# CHECK	+= $(d)/sqlite-repair.tap

CLEAN		:= $(CLEAN) store.sqlite

$(d)/sqlite-repair.tap: $(d)/sqlite-repair

$(d)/sqlite-repair: $(d)/sqlite-repair.c $(LL_store)
	$(COMPLINK) $(LL_store)

# Standard things

include end.mk
