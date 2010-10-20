# push directory on stack
include begin.mk

# objects, targets
$(OBJS) 	:= $(d)/util.o $(d)/upk_db.o $(d)/test.o
$(DEPS)		:= $(OBJS_$(d):.o=.d)
$(BIN)		:= $(d)/upk
$(LIB)		:= $(d)/store.a

$(d)/upk: $(d)/main.c $(d)/store.a deps/sqlite/sqlite3.a
	$(COMPLINK)

$(d)/store.a: $($(OBJS))
	$(ARCH)

CLEAN		:= $(CLEAN) store.sqlite

# Standard things

include end.mk
