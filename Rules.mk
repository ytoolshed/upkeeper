# Standard stuff
all:		targets


# Subdirectories, in random order
dir	:= buddy
include		$(dir)/Rules.mk
dir	:= store
include		$(dir)/Rules.mk

CLEAN	:= $(CLEAN) $(TGT_BIN) $(TGT_LIB)


# General directory independent rules

%.o:		%.c
		$(COMP)

%:		%.o
		$(LINK)

%:		%.c
		$(COMPLINK)


# The variables TGT_*, CLEAN and CMD_INST* may be added to by the Makefile
# fragments in the various subdirectories.

.PHONY:		targets
targets:	$(TGT_BIN) $(TGT_SBIN) $(TGT_ETC) $(TGT_LIB)

.PHONY:		clean
clean:
		rm -f $(CLEAN)

# Prevent make from removing any build targets, including intermediate ones

.SECONDARY:	$(CLEAN)

echo-%: 
	echo $* $($*)