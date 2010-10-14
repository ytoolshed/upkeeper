TGT_BIN		:= $(TGT_BIN) $($(BIN)) 
TGT_LIB		:= $(TGT_LIB) $($(LIB))
CLEAN		:= $(CLEAN) $($(BIN)) $($(LIB)) $($(OBJS)) $($(DEPS))

-include	$(DEPS_$(d))
d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
