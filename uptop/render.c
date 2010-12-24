#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <ncurses.h>
#include <time.h>
#include <assert.h>
#include <string.h>

struct table_item {
    char *content;
    char *format;
    int   active;
};

struct table_row {
    struct table_item *columns;
};

struct table_item *table_item_new( void );

int table_item_free(
    struct table_item *item
);

int        DEBUG = 0;

/*
 * Create a new table item
 */
struct table_item *table_item_new( void ) {
    struct table_item *item;

    item = (struct table_item *) malloc( sizeof( struct table_item ) );
    assert( item != NULL );

    item->active  = 1;
    item->content = NULL;
    item->format  = NULL;
}

/*
 * Free a table item
 */
int table_item_free(
    struct table_item *item
) {
    free( item->content );
    free( item->format );
    free( item );
}

main() {
    printf("hello\n");
}
