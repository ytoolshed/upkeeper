include 	begin.mk

$(OBJS)	:= $(d)/coe.o\
$(d)/rptqueue.o\
$(d)/sigblock.o\
$(d)/test.o\
$(d)/nonblock.o

$(DEPS)	:= $(OBJS_$(d):.o=.d)
$(LIB)	:= $(d)/common.a 

CF_$(d) += -I$(d)

CHECK	+= $(d)/nonblock.tap $(d)/sigblock.tap $(d)/coe.tap
$(BIN)	:= $(d)/nonblock.t $(d)/sigblock.t $(d)/coe.t

TESTLIB_$(d) := $($(LIB))

$(d)/common.a:   $($(OBJS))
	$(ARCH)

$(d)/nonblock.t: $(d)/nonblock.test.c $(TESTLIB_common)
	$(COMPLINK) $(TESTLIB_common)

$(d)/sigblock.t: $(d)/sigblock.test.c $(TESTLIB_common)
	$(COMPLINK) $(TESTLIB_common)

$(d)/coe.t: $(d)/coe.test.c $(TESTLIB_common)
	$(COMPLINK) $(TESTLIB_common)

# Standard things
include 	end.mk

