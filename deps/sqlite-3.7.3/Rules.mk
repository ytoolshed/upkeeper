# push directory on stack
include begin.mk

# objects, targets

$(OBJS) 	:= $(d)/sqlite3.o
$(DEPS)		:= $(OBJS_$(d):%.o=%.d)
$(LIB)		:= $(d)/sqlite3.a
$(BIN)          := $(d)/sqlite3

$(d)/sqlite3 $(d)/sqlite3.o: CF_TGT := -DSQLITE_THREADSAFE=0
$(d)/sqlite3 $(d)/sqlite3.o: CF_TGT += -DSQLITE_OMIT_LOAD_EXTENSION=1
$(d)/sqlite3.a:   $($(OBJS))
	$(ARCH)

$(d)/sqlite3: $(d)/shell.c $(d)/sqlite3.a
	$(COMPLINK) deps/sqlite/sqlite3.a


include end.mk
