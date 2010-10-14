MAIN_SRCS=main.c
MAIN_OBJS=${MAIN_SRCS:.c=.o}
MAIN=upk-buddy-controller
CFLAGS=-g
GCC_UNDEF_FLAG=-undefined dynamic_lookup

LDFLAGS=
#gcc >=4.2
#LDFLAGS=$(GCC_UNDEF_FLAG)

LIB=../store/libupk.so

test: $(MAIN)
	LD_LIBRARY_PATH=../store ./upk-buddy-controller --bootstrap
	LD_LIBRARY_PATH=../store ./upk-buddy-controller

$(MAIN): $(MAIN_OBJS) $(LIB)
	cc -o $(MAIN) $(LIB) -lsqlite3 $(MAIN_OBJS)

clean:
	rm -f *.o $(MAIN) $(LIB) store.sqlite

.SUFFIXES: .o .c

.c.o:
	$(CC) $(CFLAGS) -c $<
