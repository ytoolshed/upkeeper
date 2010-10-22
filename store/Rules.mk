# push directory on stack
include begin.mk

# objects, targets
$(OBJS) 	:= $(d)/util.o $(d)/upk_db.o $(d)/test.o
$(DEPS)		:= $(OBJS_$(d):.o=.d)
$(BIN)		:= $(d)/store-test
$(LIB)		:= $(d)/store.a

LL_$(d) := store/store.a deps/sqlite/sqlite3.a

$(d)/upk: $(d)/main.c $(d)/store.a deps/sqlite/sqlite3.a
	$(COMPLINK)

$(d)/store.a: $($(OBJS))
	$(ARCH)

CHECK	+= $(d)/store-test.t

CLEAN		:= $(CLEAN) store.sqlite

$(d)/store-test: $(d)/main.c $(LL_$(d))
	$(COMPLINK) $(LL_store)

# Standard things

include end.mk
