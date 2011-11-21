
/****************************************************************************
 * Copyright (c) 2011 Yahoo! Inc. All rights reserved. Licensed under the
 * Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License
 * at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable
 * law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 * See accompanying LICENSE file. 
 ************************************************************************** */

#include <upkeeper.h>
#include "controller.h"
#include <sqlite3.h>

sqlite3 * ctrl_dbh;

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
    sqlite3                *dbh = NULL;

    if(sqlite3_open(db_path, &dbh) != 0) {
        upk_fatal("Cannot open database");
    }
    return dbh;
}

void
upk_ctrl_exit(void)
{
    if(ctrl_dbh)
        sqlite3_close(ctrl_dbh);
    upk_ctrl_free_config();
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

    atexit(upk_ctrl_exit);
    ctrl_dbh = upk_ctrl_init_db(ctrl_db_path);
    upk_load_runtime_services();
    return 0;
}
