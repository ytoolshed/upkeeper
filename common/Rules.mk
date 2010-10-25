include 	begin.mk

$(OBJS)	:= $(d)/buffer_0.o\
$(d)/buffer_1.o\
$(d)/buffer_2.o\
$(d)/buffer.o\
$(d)/buffer_get.o\
$(d)/buffer_put.o\
$(d)/buffer_read.o\
$(d)/buffer_write.o\
$(d)/byte_chr.o\
$(d)/byte_copy.o\
$(d)/byte_cr.o\
$(d)/byte_diff.o\
$(d)/byte_rchr.o\
$(d)/coe.o\
$(d)/deepsleep.o\
$(d)/error.o\
$(d)/error_str.o\
$(d)/fd_copy.o\
$(d)/fd_move.o\
$(d)/fmt_uint0.o\
$(d)/fmt_uint.o\
$(d)/fmt_ulong.o\
$(d)/iopause.o\
$(d)/ndelay_on.o\
$(d)/rptqueue.o\
$(d)/sig_block.o\
$(d)/sig.o\
$(d)/sig_catch.o\
$(d)/sig_pause.o\
$(d)/stralloc_catb.o\
$(d)/stralloc_cat.o\
$(d)/stralloc_cats.o\
$(d)/stralloc_eady.o\
$(d)/stralloc_opyb.o\
$(d)/stralloc_opys.o\
$(d)/stralloc_pend.o\
$(d)/str_chr.o\
$(d)/str_diff.o\
$(d)/strerr_die.o\
$(d)/strerr_sys.o\
$(d)/str_len.o\
$(d)/str_start.o\
$(d)/tai64n.o\
$(d)/tai64nlocal.o\
$(d)/taia_add.o\
$(d)/taia_approx.o\
$(d)/taia_frac.o\
$(d)/taia_less.o\
$(d)/taia_now.o\
$(d)/taia_pack.o\
$(d)/taia_sub.o\
$(d)/taia_uint.o\
$(d)/tai_now.o\
$(d)/tai_pack.o\
$(d)/tai_sub.o\
$(d)/tai_unpack.o\
$(d)/timestamp.o\
$(d)/trypoll.o\
$(d)/wait_nohang.o\
$(d)/wait_pid.o

$(DEPS)	:= $(OBJS_$(d):.o=.d)
$(LIB)	:= $(d)/common.a 

$(d)/common.a:   $($(OBJS))
	$(ARCH)

# Standard things
include 	end.mk

