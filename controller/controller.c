
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
#include <sqlite3.h>
#include <sys/file.h>
#include "controller.h"
#include "ctrl_sql.h"
#include "ctrl_data.h"

sqlite3                *ctrl_dbh;
static char             flockpath[UPK_MAX_STRING_LEN] = "";
static int              flock_fd;

/** ******************************************************************************************************************
  @brief controller state initialization.
 ******************************************************************************************************************* */
void
upk_state_init(void)
{
    struct stat             st;
    int                     errno_tmp;
    int                     flock_fd = -1;

    errno = 0;
    upk_mkdir_p(upk_runtime_configuration.StateDir);
    errno = errno_tmp;
    if((stat(upk_runtime_configuration.StateDir, &st) != 0)
       || !S_ISDIR(st.st_mode)) {
        upk_fatal("State directory doesn't exist and could not be created: %s: %s\n", upk_runtime_configuration.StateDir, errno_tmp);
    }

    strncpy(flockpath, upk_runtime_configuration.StateDir, UPK_MAX_STRING_LEN - 1);
    strncat(flockpath, "/.lock", UPK_MAX_STRING_LEN - 1 - strnlen(upk_runtime_configuration.StateDir, UPK_MAX_STRING_LEN));

    errno = 0;
    flock_fd = open(flockpath, O_CREAT);
    if(errno)
        upk_fatal("Could not open %s: %s\n", flockpath, strerror(errno));

    fcntl(flock_fd, F_SETFD, FD_CLOEXEC);

    if(flock(flock_fd, LOCK_EX) != 0)
        upk_fatal("Another instance of controller is already running\n");

    return;
}

/** ******************************************************************************************************************
 ******************************************************************************************************************* */
static void
upk_db_path(char **pathbuf)
{
    strcat(*pathbuf, upk_runtime_configuration.StateDir);
    strcat(*pathbuf, "/db");

    errno = 0;
    if(mkdir(*pathbuf, 0700) != 0 && errno != EEXIST)
        upk_fatal("Cannot create database dir %s", *pathbuf);
}

/** ******************************************************************************************************************
 ******************************************************************************************************************* */
void
upk_ctrl_attach_db(sqlite3 * dbh, char *db_path, char *db_name)
{
    char                   *sqlerr;
    int                     rc = 0;
    char                    attach_str[UPK_MAX_PATH_LEN] = "";

    snprintf(attach_str, UPK_MAX_STRING_LEN, "ATTACH \"%s/%s.sqlite3\" AS \"%s\";", db_path, db_name, db_name);
    rc = sqlite3_exec(dbh, attach_str, NULL, NULL, &sqlerr);
    if(rc != SQLITE_OK) {
        upk_fatal("failed to execute: %d: %s\n", rc, sqlerr);
        sqlite3_free(sqlerr);
    }
}

/** ******************************************************************************************************************
 ******************************************************************************************************************* */
sqlite3                *
upk_ctrl_init_db(char *db_path)
{
    sqlite3                *dbh = NULL;
    int                     rc = 0;
    char                   *db_pathp = NULL;
    char                   *sqlerr = NULL;

    // char table_id[UPK_MAX_STRING_LEN] = "";

    db_pathp = db_path + strnlen(db_path, UPK_MAX_PATH_LEN);

    strcpy(db_pathp, "/upkeeper.sqlite3");
    if((rc = sqlite3_open(db_path, &dbh)) != 0) {
        upk_fatal("Cannot open database: %d\n", rc);
    }

    *db_pathp = '\0';
    upk_ctrl_attach_db(dbh, db_path, "upk_cfg");
    upk_ctrl_attach_db(dbh, db_path, "upk_audit");

    rc = sqlite3_exec(dbh, (char *) ctrl_create_audit_sql, NULL, NULL, &sqlerr);
    if(rc != SQLITE_OK) {
        upk_fatal("failed to execute: %d: %s\n", rc, sqlerr);
        sqlite3_free(sqlerr);
    }
    rc = sqlite3_exec(dbh, (char *) ctrl_create_cfg_sql, NULL, NULL, &sqlerr);
    if(rc != SQLITE_OK) {
        upk_fatal("failed to execute: %d: %s\n", rc, sqlerr);
        sqlite3_free(sqlerr);
    }

    return dbh;
}

/** ******************************************************************************************************************
 ******************************************************************************************************************* */
void
upk_ctrl_exit(void)
{
    if(ctrl_dbh)
        sqlite3_close(ctrl_dbh);
    upk_ctrl_free_config();
    close(flock_fd);
    unlink(flockpath);
}


/** ******************************************************************************************************************
 ******************************************************************************************************************* */
int
upk_ctrl_init(void)
{
    char                    ctrl_db_path[UPK_MAX_PATH_LEN] = "", *p;

    p = ctrl_db_path;

    upk_ctrl_load_config();
    upk_state_init();
    upk_db_path(&p);

    atexit(upk_ctrl_exit);
    ctrl_dbh = upk_ctrl_init_db(ctrl_db_path);
    upk_load_runtime_services();

    UPKLIST_FOREACH(upk_file_configuration.svclist) {
        upk_db_insert_cfg(ctrl_dbh, upk_file_configuration.svclist->thisp);
    }
    return 0;
}
