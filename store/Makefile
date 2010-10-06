MAIN_SRCS=main.c test.c
MAIN_OBJS=${MAIN_SRCS:.c=.o}
MAIN=upk
CFLAGS=-g
GCC_UNDEF_FLAG=-undefined dynamic_lookup

LDFLAGS=
#gcc >=4.2
#LDFLAGS=$(GCC_UNDEF_FLAG)

LIB_SRCS=upk_db.c util.c
LIB_OBJS=${LIB_SRCS:.c=.o}
LIB=libupk.so

test: $(MAIN)
	./schema-setup.sh
	LD_LIBRARY_PATH=. ./upk
	./sqlite-dump

$(LIB): $(LIB_OBJS)
	gcc -shared -lsqlite3 $(LDFLAGS) -o $(LIB) $(LIB_OBJS)

$(MAIN): $(MAIN_OBJS) $(LIB)
	cc -o $(MAIN) $(LIB) -lsqlite3 $(MAIN_OBJS)

clean:
	rm -f *.o $(MAIN) $(LIB) store.sqlite

.SUFFIXES: .o .c

.c.o:
	$(CC) $(CFLAGS) -c $<
