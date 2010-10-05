MAIN_SRCS=main.c test.c
MAIN_OBJS=${MAIN_SRCS:.c=.o}
MAIN=upk
CFLAGS=-g

LIB_SRCS=upk_db.c util.c
LIB_OBJS=${LIB_SRCS:.c=.o}
LIB=libupk.so

test: $(MAIN)
	./schema-setup.sh
	LD_LIBRARY_PATH=. ./upk
	./sqlite-dump

$(LIB): $(LIB_OBJS)
	gcc -shared -lsqlite3 -o $(LIB) $(LIB_OBJS)

$(MAIN): $(MAIN_OBJS) $(LIB)
	cc -o $(MAIN) $(LIB) -lsqlite3 $(MAIN_OBJS)

clean:
	rm -f *.o $(MAIN) $(LIB) store.sqlite

.SUFFIXES: .o .c

.c.o:
	$(CC) $(CFLAGS) -c $<
