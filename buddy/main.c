#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "../store/upk_db.h"
#include "upk_buddy.h"

int DEBUG = 0;

int main( 
    int   argc, 
    char *argv[] 
) {
    sqlite3 *pdb;
    char    *file = "store.sqlite";
    int      rc;
    int      i;
    char    *event;
    char    *status;
    char    *service;
    char    *package;
    char    *cp;

    printf("Waaaah!\n");
    return(0);
}
