# push directory on stack
include begin.mk

# objects, targets
$(OBJS) 	:= $(d)/util.o $(d)/upk_db.o 
$(DEPS)		:= $(OBJS_$(d):.o=.d)
$(BIN)		:= $(d)/store-test
$(LIB)		:= $(d)/store.a

LL_$(d) := store/store.a deps/sqlite/sqlite3.a common/common.a

$(d)/upk: $(d)/main.o $(d)/store.a deps/sqlite/sqlite3.a
	$(COMPLINK)

$(d)/store.a: $($(OBJS))
	$(ARCH)


CHECK	+= $(d)/store-test.tap

CLEAN		:= $(CLEAN) store.sqlite 

$(d)/store-test.tap: $(d)/store-test

$(d)/store-test: $(d)/main.c $(LL_store)
	$(COMPLINK) $(LL_store)

# Standard things

include end.mk
