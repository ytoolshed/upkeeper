include 	begin.mk


$(OBJS)	:= $(d)/upk_buddy.o $(d)/sigblock.o
$(DEPS)	:= $(OBJS_$(d):.o=.d)

CLEAN	:= $(CLEAN) $(OBJS_$(d)) $(DEPS_$(d))

$(BIN)	:= $(d)/upkb
$(LIB)	:= $(d)/buddy.a 

$(d)/buddy.a: $($(OBJS))
	$(ARCH)

$(d)/upkb: $(d)/main.o store/store.a $(d)/buddy.a deps/sqlite/sqlite3.a
	$(LINK)

# Standard things
include 	end.mk

