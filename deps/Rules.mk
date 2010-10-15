# push directory on stack
include begin.mk

dir	:= $(d)/sqlite
include	$(dir)/Rules.mk

# Standard things

include end.mk
