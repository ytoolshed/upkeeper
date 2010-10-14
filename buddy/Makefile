MAIN_SRCS=main.c
MAIN_OBJS=${MAIN_SRCS:.c=.o}
MAIN=buddytest
CFLAGS=-g
GCC_UNDEF_FLAG=-undefined dynamic_lookup
EXT_LIBS=../store/libupk.so

LDFLAGS=
#gcc >=4.2
#LDFLAGS=$(GCC_UNDEF_FLAG)

LIB_SRCS=upk_buddy.c
LIB_OBJS=${LIB_SRCS:.c=.o}
LIB=libupk_buddy.so

test: $(MAIN)
	LD_LIBRARY_PATH=. ./buddytest

$(LIB): $(LIB_OBJS)
	gcc -shared -lsqlite3 $(LDFLAGS) -o $(LIB) $(LIB_OBJS)

$(MAIN): $(MAIN_OBJS) $(LIB)
	cc -o $(MAIN) $(LIB) -lsqlite3 $(MAIN_OBJS) $(EXT_LIBS)

clean:
	rm -f *.o $(MAIN) $(LIB)

.SUFFIXES: .o .c

.c.o:
	$(CC) $(CFLAGS) -c $<
