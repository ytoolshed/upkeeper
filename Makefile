### Build flags for all targets
#
CFLAGS          = -g -Wall -I. -MD
LDFLAGS         = 
LDLIBS          = -lsqlite3

### Build tools
# 
COMP            = $(CC) $(CFLAGS) $(CF_TGT) -o $@ -c $<
LINK            = $(CC) $(LDFLAGS) $(LF_TGT) -o $@ $^ $(LL_TGT) $(LDLIBS)
COMPLINK        = $(CC) $(CFLAGS) $(CF_TGT) $(LDFLAGS) $(LF_TGT) -o $@ $< $(LL_TGT) $(LDLIBS)
ARCH		= $(AR) $(ARFLAGS) $@ $^

### Standard parts
#
include Rules.mk

