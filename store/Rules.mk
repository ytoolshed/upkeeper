# push directory on stack
include begin.mk

# objects, targets
$(OBJS) 	:= $(d)/util.o $(d)/upk_db.o $(d)/test.o
$(DEPS)		:= $(OBJS_$(d):.o=.d)
$(BIN)		:= $(d)/upk
$(LIB)		:= $(d)/store.a

$(d)/upk_db.o: $(d)/schema.c

$(d)/upk: $(d)/main.c $(d)/store.a deps/sqlite/sqlite3.a
	$(COMPLINK)

$(d)/store.a: $($(OBJS))
	$(ARCH)


$(d)/schema.c: $(d)/schema.sql
	echo 'const char *schema = ' > $@
	sed -e 's/^/"/' -e 's/$$/"/' < $< >> $@
	echo ';' >> $@

CLEAN		:= $(CLEAN) store.sqlite $(d)/schema.c

# Standard things

include end.mk
