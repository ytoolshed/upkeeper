#include <upkeeper.h>
#include "controller.h"
#include <sqlite3.h>

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
static void
upk_db_path(char **pathbuf)
{
    strcat(*pathbuf, upk_runtime_configuration.StateDir);
    strcat(*pathbuf, "/db");

    errno = 0;
    if(mkdir(*pathbuf, 0700) != 0 && errno != EEXIST)
        upk_fatal("Cannot create database dir %s", *pathbuf);

    strcat(*pathbuf, "/upkeeper.sqlite3");
}

/* ******************************************************************************************************************
   ****************************************************************************************************************** */
sqlite3                *
upk_ctrl_init_db(const char *db_path)
{
    sqlite3                *dbh;

    if(sqlite3_open(db_path, &dbh) != 0) {
        upk_fatal("Cannot open database");
    }
    return dbh;
}


/* ******************************************************************************************************************
   ****************************************************************************************************************** */
int
upk_ctrl_init(void)
{
    char                    ctrl_db_path[UPK_MAX_PATH_LEN] = "", *p;

    p = ctrl_db_path;

    upk_ctrl_load_config();
    upk_db_path(&p);

    upk_ctrl_init_db(ctrl_db_path);
    upk_load_runtime_services();
    return 0;
}
