# push directory on stack
include begin.mk

# objects, targets
CF_TGT		:= -DSQLITE_THREADSAFE=0 
LF_TGT		:= -ldl
$(OBJS) 	:= $(d)/sqlite3.o
$(DEPS)		:= $(OBJS_$(d):%.o=%.d)
$(LIB)		:= $(d)/sqlite3.a


$(d)/sqlite3.a:   $($(OBJS))
		$(ARCH)

include end.mk
