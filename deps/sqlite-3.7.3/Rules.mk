# push directory on stack
include begin.mk

# objects, targets

$(OBJS) 	:= $(d)/sqlite3.o
$(DEPS)		:= $(OBJS_$(d):%.o=%.d)
$(LIB)		:= $(d)/sqlite3.a

$(d)/sqlite3.o: CF_TGT		:= -DSQLITE_THREADSAFE=0 
$(d)/sqlite3.a:   $($(OBJS))
		$(ARCH)

include end.mk
