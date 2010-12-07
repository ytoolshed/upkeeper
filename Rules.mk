CHECK := $(CHECK)
# Subdirectories, in random order
dir	:= buddy
include		$(dir)/Rules.mk
dir	:= store
include		$(dir)/Rules.mk
dir	:= deps
include		$(dir)/Rules.mk
dir	:= common
include		$(dir)/Rules.mk
dir	:= controller
include		$(dir)/Rules.mk
dir	:= uptop
include		$(dir)/Rules.mk

DEPS_LEFTOVERS=*/*.dSYM

CLEAN	:= $(CLEAN) $(TGT_BIN) $(TGT_LIB) $(TGT_DEPS) $(CHECK)


# The variables TGT_*, CHECK, CLEAN and CMD_INST* may be added to by the Makefile
# fragments in the various subdirectories.
# The variables

.PHONY:		targets
targets:	$(TGT_BIN) $(TGT_SBIN) $(TGT_ETC) $(TGT_LIB)

.PHONY:		clean
clean:
		rm -f $(CLEAN)
		rm -rf $(DEPS_LEFTOVERS)

check:		$(CHECK)
t-files:	
	echo $(CHECK)


install:
# Prevent make from removing any build targets, including intermediate ones

.SECONDARY:	$(CLEAN)

.t.tap:
	exec $< > $@

echo-%: 
	echo $* $($*)


.c.o: 	
	$(COMP)

