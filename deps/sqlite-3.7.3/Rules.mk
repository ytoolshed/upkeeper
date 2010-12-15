# push directory on stack
include begin.mk

# objects, targets

$(OBJS) 	:= $(d)/sqlite3.o
$(DEPS)		:= $(OBJS_$(d):%.o=%.d)
$(LIB)		:= $(d)/sqlite3.a

$(d)/sqlite3.o: CF_TGT		:= -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION=1
$(d)/sqlite3.a:   $($(OBJS))
		$(ARCH)

include end.mk
