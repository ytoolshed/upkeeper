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

void table_item_format_set(
    struct table_item *item,
    char *format
);

void table_item_format_set(
    struct table_item *item,
    char *format
);

void table_item_free(
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
 * Dump the content of a table item
 */
void table_item_dump(
    struct table_item *item
) {
    if( item == NULL ) {
        printf( "table_item: NULL\n" );
        return;
    }

    printf( "table_item: content='%s' format='%s'\n",
            item->content == NULL ? "NULL" : item->content,
            item->format  == NULL ? "NULL" : item->format
          );

    return;
}

/*
 * Set the content of a table item
 */
void table_item_content_set(
    struct table_item *item,
    char *content
) {
    char *dup = strdup( content );
    assert( dup != NULL );

    item->content = dup;
}

/*
 * Set the format of a table item
 */
void table_item_format_set(
    struct table_item *item,
    char *format
) {
    char *dup = strdup( format );
    assert( dup != NULL );

    item->format = dup;
}

/*
 * Free a table item
 */
void table_item_free(
    struct table_item *item
) {
    if( item == NULL )
        return;

    if( item->content != NULL )
        free( item->content );

    if( item->format != NULL )
        free( item->format );

    free( item );

    return;
}

main() {
    struct table_item *item;

    item = table_item_new();

    table_item_format_set( item, "%05d" );
    table_item_content_set( item, "123" );
    table_item_dump( item );

    table_item_free( item );
}
