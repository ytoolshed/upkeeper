TGT_BIN		:= $(TGT_BIN) $($(BIN)) 
TGT_LIB		:= $(TGT_LIB) $($(LIB))
TGT_DEPS	:= $($(BIN)) $($(DEPS)) $(TGT_DEPS)
CLEAN		:= $(CLEAN) $($(BIN):%=%.d) $($(LIB)) $($(OBJS))

-include	$(DEPS_$(d))
d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
