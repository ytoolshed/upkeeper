include 	begin.mk

$(OBJS)	:= $(d)/coe.o\
$(d)/rptqueue.o\
$(d)/sigblock.o\
$(d)/nonblock.o

$(DEPS)	:= $(OBJS_$(d):.o=.d)
$(LIB)	:= $(d)/common.a 

$(d)/common.a:   $($(OBJS))
	$(ARCH)

# Standard things
include 	end.mk

