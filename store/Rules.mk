# push directory on stack
include begin.mk

# objects, targets
$(OBJS) 	:= $(d)/upk_db.o $(d)/util.o $(d)/test.o
$(DEPS)		:= $(OBJS_$(d):%.o=%.d)
$(BIN)		:= $(d)/upk
$(LIB)		:= $(d)/store.a


$(d)/store.a:   $($(OBJS))
		$(ARCH)

$(d)/upk: $(d)/main.o $(d)/store.a
		$(LINK) 

CLEAN		:= $(CLEAN) store.sqlite

# Standard things

include end.mk