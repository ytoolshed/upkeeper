
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

/**
  @addtogroup controller
  @{
  */

/** ******************************************************************************************************************
  @brief lookup a table_id from the schema table.

  @param[in] dbh                    The database handle.
  @param[out] value                 The place to store the single value.
  @param[in] sql                    The table to query.
  @param[in] ...                    Values to substitute via bind (terminate with (char *) NULL) - If there are 0 
                                    arguments, you must still include a (char *) NULL).

  @return 0 on success, -1 if table-name not found, and the result code if there was a sql error
 ******************************************************************************************************************* */
int
upk_db_get_single_text(sqlite3 * dbh, char *value, const char *sql, ...)
{
    va_list                 ap;
    int                     rc = 0, ret = 0, idx = 1;
    sqlite3_stmt           *stmt = NULL;
    char                   *res = NULL, *arg = NULL;

    va_start(ap, sql);

    ret = rc = sqlite3_prepare(dbh, sql, UPK_MAX_STRING_LEN, &stmt, NULL);
    UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK && stmt), UPK_SQL_ERROR, "prepare: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);

    while((arg = va_arg(ap, char *))) {
        upk_debug1("query: %s bind arg: %s\n", sql, arg);
        ret = rc = sqlite3_bind_text(stmt, idx++, arg, -1, SQLITE_TRANSIENT);
        UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);
    }

    ret = rc = sqlite3_step(stmt);
    UPK_FUNC_ASSERT_MSG((rc == SQLITE_ROW
                         || rc == SQLITE_DONE), UPK_SQL_ERROR, "SQLite encountered the following error executing %s: %s (%d)\n", sql,
                        sqlite3_errmsg(dbh), rc);
    ret = -1;

    if(rc == SQLITE_ROW) {
        res = (char *) sqlite3_column_text(stmt, 0);
        if(res) {
            strncpy(value, res, UPK_MAX_STRING_LEN - 1);
            ret = 0;
        }
    }

    IF_UPK_ERROR {
        if(stmt) {
            rc = sqlite3_reset(stmt);
            ret = (ret == 0) ? ret : rc;
            if(rc != SQLITE_OK)
                upk_error("SQLite Error: %s (%d)\n", sqlite3_errmsg(dbh), rc);
        }
    }

    va_end(ap);
    sqlite3_finalize(stmt);
    return ret;
}



/** ******************************************************************************************************************
  @brief lookup a table_id from the schema table.

  @param[in] dbh                    The database handle.
  @param[in] db                     The name of the database to search in.
  @param[in] table_name             The name of the table you are lookin for.
  @param[out] table_id              The result of looking up the table id.

  @return 0 on success, -1 if table-name not found, and the result code if there was a sql error
 ******************************************************************************************************************* */
static int
upk_db_get_table_id(sqlite3 * dbh, const char *db, const char *table_name, char *table_id)
{
    char                    query_str[UPK_MAX_STRING_LEN] = "";

    snprintf(query_str, UPK_MAX_STRING_LEN - 1, "SELECT (table_id) FROM %s._schema WHERE table_name = ?;", db);
    return upk_db_get_single_text(dbh, table_id, query_str, table_name, (char *) NULL);
}

/** ******************************************************************************************************************
  @brief lookup the table_id for upk_audit.service from the schema table.

  @param[in] dbh                    The database handle.
  @param[out] table_id              The result of looking up the table id.

  @return 0 on success, -1 if table-name not found, and the result code if there was a sql error
 ******************************************************************************************************************* */
int
upk_db_get_audit_service_table(sqlite3 * dbh, char *table_id)
{
    return upk_db_get_table_id(dbh, "upk_audit", "service", table_id);
}


/** ******************************************************************************************************************
  @brief lookup the table_id for upk_cfg.services from the schema table.

  @param[in] dbh                    The database handle.
  @param[out] table_id              The result of looking up the table id.

  @return 0 on success, -1 if table-name not found, and the result code if there was a sql error
 ******************************************************************************************************************* */
int
upk_db_get_cfg_services_table(sqlite3 * dbh, char *table_id)
{
    return upk_db_get_table_id(dbh, "upk_cfg", "services", table_id);
}


/** ******************************************************************************************************************
  @brief lookup the table_id for upk_cfg.svc_optionsfrom the schema table.

  @param[in] dbh                    The database handle.
  @param[out] table_id              The result of looking up the table id.

  @return 0 on success, -1 if table-name not found, and the result code if there was a sql error
 ******************************************************************************************************************* */
int
upk_db_get_cfg_svc_options_table(sqlite3 * dbh, char *table_id)
{
    return upk_db_get_table_id(dbh, "upk_cfg", "svc_options", table_id);
}

/** ******************************************************************************************************************
  @brief lookup the table_id for upk_cfg.svc_list_options from the schema table.

  @param[in] dbh                    The database handle.
  @param[out] table_id              The result of looking up the table id.

  @return 0 on success, -1 if table-name not found, and the result code if there was a sql error
 ******************************************************************************************************************* */
int
upk_db_get_cfg_svc_list_options_table(sqlite3 * dbh, char *table_id)
{
    return upk_db_get_table_id(dbh, "upk_cfg", "svc_list_options", table_id);
}


/** ******************************************************************************************************************
  @brief lookup a service <package>-<name>

  @param[in] dbh                    The database handle.
  @param[in] svc_id                 The service description to lookup.
  @param[in] package                The package to lookup.

  @return the UUID of the service if found, NULL if not found.
 ******************************************************************************************************************* */
char                   *
upk_db_svc_uuid_lookup(sqlite3 * dbh, const char *svcid, const char *package)
{
    char                    query_str[UPK_MAX_STRING_LEN] = "";
    char                    table_id[UPK_MAX_STRING_LEN] = "";
    static char             uuid[UPK_MAX_STRING_LEN] = "";                               /* UPK_MAX_STRING_LEN instead of
                                                                                            UPK_UUID_STRING_LEN because
                                                                                            upk_db_get_single_text will write
                                                                                            UPK_MAX_STRING_LEN bytes */

    upk_db_get_cfg_services_table(dbh, table_id);

    snprintf(query_str, UPK_MAX_STRING_LEN - 1, "SELECT (uuid) FROM upk_cfg.%s WHERE (svcid = ?%s);", table_id,
             ((package) ? " AND package = ?" : " AND package = NULL"));
    upk_db_get_single_text(dbh, uuid, query_str, svcid, package, (char *) NULL);

    if(strnlen(uuid, 2) > 0)
        return uuid;

    return NULL;
}

/** ******************************************************************************************************************
  @brief generate new unique service UUID.

  @param[in] dbh                    The database handle.

  @return uuid
 ******************************************************************************************************************* */
char                   *
upk_db_gen_uuid(sqlite3 * dbh)
{
    int                     fd = -1;
    char                    matching_uuid[UPK_MAX_STRING_LEN] = "";
    static char             uuid_str[UPK_UUID_STRING_LEN + 1] = "";
    char                    query_str[UPK_MAX_STRING_LEN] = "";
    char                    table_id[UPK_MAX_STRING_LEN] = "";
    upk_uuid_t              uuid = { 0 };

    upk_db_get_cfg_services_table(dbh, table_id);
    snprintf(query_str, UPK_MAX_STRING_LEN - 1, "SELECT (uuid) FROM upk_cfg.%s WHERE (uuid = ?);", table_id);

    fd = upk_uuid_open_random();
    if(fd < 0)
        upk_uuid_seed_random();

    do {
        upk_gen_uuid_bytes(&uuid);
        upk_uuid_to_string(uuid_str, &uuid);

        upk_db_get_single_text(dbh, matching_uuid, query_str, uuid_str, (char *) NULL);
    } while(strnlen(matching_uuid, 2) > 0);

    return uuid_str;
}



/** ******************************************************************************************************************
  @brief Create new service definition.

  @param[in] dbh                    The database handle.
  @param[in] svc                    The service description to insert.

  @return true on success, false on failure.
 ******************************************************************************************************************* */
bool
upk_db_add_new_service(sqlite3 * dbh, upk_svc_desc_t * svc)
{
    int                     rc = 0;
    char                    uuid[UPK_UUID_STRING_LEN + 1] = "";
    char                    svcid[UPK_MAX_STRING_LEN] = "";
    char                    table_id[UPK_MAX_STRING_LEN] = "";
    char                    matching_uuid[UPK_MAX_STRING_LEN] = "";
    char                   *sqlerr = NULL;
    char                    query_str[UPK_MAX_STRING_LEN] = "";

    upk_db_get_cfg_services_table(dbh, table_id);
    upk_concat_svcid(svcid, svc->Package, svc->Name);

    upk_uuid_to_string(uuid, &svc->UUID);
    if(strcmp(uuid, "00000000-0000-0000-0000-000000000000") == 0) {
        strncpy(uuid, upk_db_gen_uuid(dbh), UPK_UUID_STRING_LEN);
        upk_string_to_uuid(&svc->UUID, uuid);                                            /* handled later in
                                                                                            upk_db_populate_runtime_config, but done here
                                                                                            just to have it identifiable */
    } else {
        snprintf(query_str, UPK_MAX_STRING_LEN - 1, "SELECT (uuid) FROM upk_cfg.%s WHERE (uuid = ?);", table_id);
        upk_db_get_single_text(dbh, matching_uuid, query_str, uuid, (char *) NULL);
        UPK_FUNC_ASSERT_MSG((strcmp(matching_uuid, uuid) != 0), UPK_CONF_ERROR, "Conflicting UUID in data store: %s for %s\n", uuid, svcid);
    }

    snprintf(query_str, UPK_MAX_STRING_LEN - 1,
             "INSERT INTO upk_cfg.%s (uuid, svcid, is_active, utc_created, utc_modified, utc_deleted) VALUES"
             "(\"%s\",\"%s\",1,%ld,%ld,NULL);", table_id, uuid, svc->Name, time(NULL), time(NULL));
    rc = sqlite3_exec(dbh, query_str, NULL, NULL, &sqlerr);
    UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "Unable to add %s to data store: %s", svcid, sqlerr);

    snprintf(query_str, UPK_MAX_STRING_LEN - 1, "UPDATE upk_cfg.%s SET package = \"%s\" WHERE uuid = \"%s\";", table_id, svc->Package,
             uuid);
    rc = sqlite3_exec(dbh, query_str, NULL, NULL, &sqlerr);
    UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "Unable to update Package for %s to data store: %s", svcid, sqlerr);

    IF_UPK_ERROR {
        if(sqlerr)
            sqlite3_free(sqlerr);
        return false;
    }
    return true;
}

/** ******************************************************************************************************************
  @brief try to insert an option

  @param[in] dbh                    The database handle.
  @param[in] svc                    The service description to insert.
  @param[in] type                   The type to use for bind.
  @param[in] optkey                 The name of the option to insert/update.
  @param[in] optval                 The value to insert/update.

  @return 0 on success, ! 0 on error.
 ******************************************************************************************************************* */
static inline int
upk_db_try_insert(sqlite3 * dbh, char *table_id, upk_svc_desc_t * svc, int type, char *optkey, void *optval, size_t len, char *section,
                  char *id)
{
    char                    sql[UPK_MAX_STRING_LEN] = "";

    // char table_id[UPK_MAX_STRING_LEN] = "";
    char                    uuid_str[UPK_UUID_STRING_LEN + 1] = "";
    int                     rc = 0, ret = 0, idx = 1;
    sqlite3_stmt           *stmt = NULL;

    upk_uuid_to_string(uuid_str, &svc->UUID);

    snprintf(sql, UPK_MAX_STRING_LEN - 1,
             "INSERT INTO upk_cfg.%s (uuid, key, value, utc_added, utc_modified%s%s%s%s) VALUES (?, ?, ?, ?, ?%s%s);", table_id,
             (section) ? ", " : "", (section) ? "section" : "", (id) ? ", " : "", (id) ? "id" : "", (section) ? ", ?" : "",
             (id) ? ", ?" : "");
    upk_debug1("%s\n", sql);

    ret = rc = sqlite3_prepare(dbh, sql, UPK_MAX_STRING_LEN, &stmt, NULL);
    UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK && stmt), UPK_SQL_ERROR, "prepare: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);

    ret = rc = sqlite3_bind_text(stmt, idx++, uuid_str, -1, SQLITE_TRANSIENT);
    UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);

    ret = rc = sqlite3_bind_text(stmt, idx++, optkey, -1, SQLITE_TRANSIENT);
    UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);

    switch (type) {
    case SQLITE_INTEGER:
        ret = rc = sqlite3_bind_int(stmt, idx++, *((int *) optval));
        UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);
        break;
    case SQLITE_FLOAT:
        ret = rc = sqlite3_bind_double(stmt, idx++, *((double *) optval));
        UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);
        break;
    case SQLITE_BLOB:
        ret = rc = sqlite3_bind_blob(stmt, idx++, optval, len, SQLITE_TRANSIENT);
        UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);
        break;
    case SQLITE_TEXT:
        ret = rc = sqlite3_bind_text(stmt, idx++, (char *) optval, len, SQLITE_TRANSIENT);
        UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);
        break;
    case SQLITE_NULL:
        ret = rc = sqlite3_bind_null(stmt, idx++);
        UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);
        break;
    }

    ret = rc = sqlite3_bind_int(stmt, idx++, time(NULL));
    UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);

    ret = rc = sqlite3_bind_int(stmt, idx++, time(NULL));
    UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);

    if(section) {
        ret = rc = sqlite3_bind_text(stmt, idx++, section, -1, SQLITE_TRANSIENT);
        UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);
    }
    if(id) {
        ret = rc = sqlite3_bind_text(stmt, idx++, id, -1, SQLITE_TRANSIENT);
        UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);
    }

    ret = rc = sqlite3_step(stmt);
    if(rc == SQLITE_DONE) {
        ret = 0;
    } else if(rc == SQLITE_ERROR) {                                                      /* SQLITE_ERROR may be SQLITE_CONSTRAINT
                                                                                            violation, which probably means "not-unique
                                                                                            primary key composite", which is fine, because
                                                                                            we will attempt to update in this case; for all 
                                                                                            other errors however, we do want them reported */
        rc = sqlite3_reset(stmt);
        UPK_FUNC_ASSERT_MSG((rc == SQLITE_CONSTRAINT), UPK_SQL_ERROR, "SQLite encountered the following error executing %s: %s (%d)\n", sql,
                            sqlite3_errmsg(dbh), rc);
    }

    UPK_FUNC_ASSERT_MSG((rc != SQLITE_MISUSE), UPK_SQL_ERROR, "SQLite encountered the following error executing %s: %s (%d)\n", sql,
                        sqlite3_errmsg(dbh), rc);

    IF_UPK_ERROR {
        if(stmt) {
            rc = sqlite3_reset(stmt);
            ret = (ret == 0) ? ret : rc;
            if(rc != SQLITE_OK)
                upk_error("SQLite Error: %s (%d)\n", sqlite3_errmsg(dbh), rc);
        }
    }

    sqlite3_finalize(stmt);
    return ret;
}

/** ******************************************************************************************************************
  @brief try to update an option

  @param[in] dbh                    The database handle.
  @param[in] svc                    The service description to insert.
  @param[in] type                   The type to use for bind.
  @param[in] optkey                 The name of the option to insert/update.
  @param[in] optval                 The value to insert/update.

  @return 0 on success, ! 0 on error.
 ******************************************************************************************************************* */
static inline int
upk_db_try_update(sqlite3 * dbh, char *table_id, upk_svc_desc_t * svc, int type, char *optkey, void *optval, size_t len, char *section,
                  char *id)
{
    char                    sql[UPK_MAX_STRING_LEN] = "";

    // char table_id[UPK_MAX_STRING_LEN] = "";
    char                    uuid_str[UPK_UUID_STRING_LEN + 1] = "";
    int                     rc = 0, ret = 0;
    sqlite3_stmt           *stmt = NULL;

    upk_uuid_to_string(uuid_str, &svc->UUID);

    snprintf(sql, UPK_MAX_STRING_LEN - 1, "UPDATE upk_cfg.%s SET value = ?, utc_modified = ? WHERE uuid = ? AND key = ?;", table_id);

    ret = rc = sqlite3_prepare(dbh, sql, UPK_MAX_STRING_LEN, &stmt, NULL);
    UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK && stmt), UPK_SQL_ERROR, "prepare: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);

    switch (type) {
    case SQLITE_INTEGER:
        ret = rc = sqlite3_bind_int(stmt, 1, *((int *) optval));
        UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);
        break;
    case SQLITE_FLOAT:
        ret = rc = sqlite3_bind_double(stmt, 1, *((double *) optval));
        UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);
        break;
    case SQLITE_BLOB:
        ret = rc = sqlite3_bind_blob(stmt, 1, optval, len, SQLITE_TRANSIENT);
        UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);
        break;
    case SQLITE_TEXT:
        ret = rc = sqlite3_bind_text(stmt, 1, (char *) optval, len, SQLITE_TRANSIENT);
        UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);
        break;
    case SQLITE_NULL:
        ret = rc = sqlite3_bind_null(stmt, 1);
        UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);
        break;
    }

    ret = rc = sqlite3_bind_int(stmt, 2, time(NULL));
    UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);

    ret = rc = sqlite3_bind_text(stmt, 3, uuid_str, -1, SQLITE_TRANSIENT);
    UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);

    ret = rc = sqlite3_bind_text(stmt, 4, optkey, -1, SQLITE_TRANSIENT);
    UPK_FUNC_ASSERT_MSG((rc == SQLITE_OK), UPK_SQL_ERROR, "bind: %s: %s (%d)\n", sql, sqlite3_errmsg(dbh), rc);

    ret = rc = sqlite3_step(stmt);
    if(rc == SQLITE_DONE) {
        ret = 0;
    }

    UPK_FUNC_ASSERT_MSG((rc != SQLITE_ERROR
                         && rc != SQLITE_MISUSE), UPK_SQL_ERROR, "SQLite encountered the following error executing %s: %s (%d)\n", sql,
                        sqlite3_errmsg(dbh), rc);


    IF_UPK_ERROR {
        if(stmt) {
            rc = sqlite3_reset(stmt);
            ret = (ret == 0) ? ret : rc;
            if(rc != SQLITE_OK)
                upk_error("SQLite Error: %s (%d)\n", sqlite3_errmsg(dbh), rc);
        }
    }

    sqlite3_finalize(stmt);
    return ret;
}

/** ******************************************************************************************************************
  @brief insert or update a option.

  @param[in] dbh                    The database handle.
  @param[in] svc                    The service description to insert.
  @param[in] type                   The type to use for bind.
  @param[in] optkey                 The name of the option to insert/update.
  @param[in] optval                 The value to insert/update.

  @return true on success, false on failure.
 ******************************************************************************************************************* */
bool
upk_db_upsert(sqlite3 * dbh, char *table_id, upk_svc_desc_t * svc, int type, char *optkey, void *optval, size_t len, char *section,
              char *id)
{

    char                    svcid[UPK_MAX_STRING_LEN] = "";
    char                    uuid[UPK_UUID_STRING_LEN + 1] = "";

    if(upk_db_try_insert(dbh, table_id, svc, type, optkey, optval, len, section, id) != 0) {
        if(upk_db_try_update(dbh, table_id, svc, type, optkey, optval, len, section, id) != 0) {
            upk_concat_svcid(svcid, svc->Package, svc->Name);
            upk_uuid_to_string(uuid, &svc->UUID);
            upk_verbose("could not upsert %s for %s (%s)\n", optkey, svcid, uuid);
            return false;
        }
    }
    return true;
}

/** ******************************************************************************************************************
  @brief insert or update a list option.

  @param[in] dbh                    The database handle.
  @param[in] svc                    The service description to insert.
  @param[in] type                   The type to use for bind.
  @param[in] optkey                 The name of the option to insert/update.
  @param[in] optval                 The value to insert/update.

  @return true on success, false on failure.
 ******************************************************************************************************************* */


#define UPSERT_TEXT(A) \
        do { \
            if(svc->A && strnlen(svc->A, UPK_MAX_STRING_LEN) > 0) { \
                n = strnlen(svc->A, UPK_MAX_STRING_LEN); \
                upk_db_upsert(dbh, table_id, svc, SQLITE_TEXT, stringify(A), svc->A, -1, NULL, NULL); \
                hash_buffer_len += n; \
                hash_buffer = realloc(hash_buffer, hash_buffer_len); \
                strncat(hash_buffer, svc->A, n); \
            } \
        } while(0)

#define UPSERT_INT(A, NMIN) \
        do { \
            if(svc->A != NMIN) { \
                upk_db_upsert(dbh, table_id, svc, SQLITE_INTEGER, stringify(A), &svc->A, 0, NULL, NULL); \
                n = snprintf(tmp_buffer, UPK_MAX_STRING_LEN - 1, "%ld", (long int) svc->A); \
                if(n > 0) { \
                    hash_buffer_len += n;  \
                    hash_buffer = realloc(hash_buffer, hash_buffer_len); \
                    strncat(hash_buffer, tmp_buffer, n); \
                } \
            } \
        } while(0)

#define LIST_UPSERT_TEXT(A,B,C) \
        do { \
            if(svc->A && svc->A->thisp && svc->A->thisp->B && strnlen(svc->A->thisp->B, UPK_MAX_STRING_LEN) > 0) { \
                n = strnlen(svc->A->thisp->B, UPK_MAX_STRING_LEN); \
                upk_db_upsert(dbh, table_id, svc, SQLITE_TEXT, stringify(B), svc->A->thisp->B, -1, stringify(A), C); \
                if(n > 0) { \
                    hash_buffer_len += n; \
                    hash_buffer = realloc(hash_buffer, hash_buffer_len); \
                    strncat(hash_buffer, svc->A->thisp->B, n); \
                } \
            } \
        } while(0)



/** ******************************************************************************************************************
  @brief Insert options into database;

  @param[in] dbh                    The database handle.
  @param[in] svc                    The service description to insert.

  @return the hash of the added options on success, 0 on failure.
 ******************************************************************************************************************* */
int32_t
upk_db_upsert_options(sqlite3 * dbh, upk_svc_desc_t * svc)
{
    char                   *hash_buffer = calloc(1, 1);
    size_t                  hash_buffer_len = 1, n = 0, idx = 0;
    char                    tmp_buffer[UPK_MAX_STRING_LEN] = "";
    int32_t                 hash = 0;
    char                    table_id[UPK_MAX_STRING_LEN] = "";
    char                    sql[UPK_MAX_STRING_LEN] = "";
    char                    uuid[UPK_MAX_STRING_LEN] = "";
    char                   *sqlerr;
    int                     rc = 0;

    /* FIXME: check hash before blindly nuking all existing list_items and repopulating from file */

    upk_db_get_cfg_svc_options_table(dbh, table_id);

    UPSERT_INT(StartPriority, INT32_MIN);
    UPSERT_TEXT(Name);
    UPSERT_TEXT(Package);
    UPSERT_TEXT(ShortDescription);
    UPSERT_TEXT(LongDescription);
    UPSERT_INT(RestartAllDependencies, INT8_MIN);
    UPSERT_INT(RestartOnPrerequisiteRestart, INT8_MIN);
    UPSERT_INT(KillTimeout, INT32_MIN);
    UPSERT_INT(StartupDelay, INT32_MIN);
    UPSERT_INT(MaxConsecutiveFailures, INT32_MIN);
    UPSERT_INT(UserMaxRestarts, INT32_MIN);
    UPSERT_INT(UserRestartWindow, INT32_MIN);
    UPSERT_INT(UserRateLimit, INT32_MIN);
    UPSERT_INT(RandomizeRateLimit, INT8_MIN);
    UPSERT_INT(SetUID, INT32_MIN);
    UPSERT_INT(SetGID, INT32_MIN);
    UPSERT_INT(RingbufferSize, INT32_MIN);
    UPSERT_INT(ReconnectRetries, INT32_MIN);
    UPSERT_TEXT(ExecStart);
    UPSERT_TEXT(StartScript);
    UPSERT_TEXT(ExecStop);
    UPSERT_TEXT(StopScript);
    UPSERT_TEXT(ExecReload);
    UPSERT_TEXT(ReloadScript);
    UPSERT_TEXT(PipeStdoutScript);
    UPSERT_TEXT(PipeStderrScript);
    UPSERT_TEXT(RedirectStdout);
    UPSERT_TEXT(RedirectStderr);
    UPSERT_INT(InitialState, INT32_MIN);
    UPSERT_INT(UnconfigureOnFileRemoval, INT8_MIN);
    UPSERT_INT(PreferBuddyStateForStopped, INT8_MIN);
    UPSERT_INT(PreferBuddyStateForRunning, INT8_MIN);

    hash = upk_crc32((unsigned char *) hash_buffer, hash_buffer_len);

    upk_db_get_cfg_svc_list_options_table(dbh, table_id);
    upk_uuid_to_string(uuid, &svc->UUID);

    snprintf(sql, UPK_MAX_STRING_LEN, "DELETE FROM upk_cfg.%s WHERE uuid = \"%s\";", table_id, uuid);
    rc = sqlite3_exec(dbh, sql, NULL, NULL, &sqlerr);
    if(rc != SQLITE_OK) {
        upk_debug1("delete errore: %s for %s\n", sqlerr, sql);
        sqlite3_free(sqlerr);
    }


    idx = 0;
    if(svc->Provides && svc->Provides->count > 0) {
        UPKLIST_FOREACH(svc->Provides) {
            snprintf(tmp_buffer, UPK_MAX_STRING_LEN, "%ld", idx++);
            LIST_UPSERT_TEXT(Provides, name, tmp_buffer);
            LIST_UPSERT_TEXT(Provides, pkg, tmp_buffer);
        }
    }

    /* FIXME: needs to handle UUID */
    idx = 0;
    if(svc->StartAfter && svc->StartAfter->count > 0) {
        UPKLIST_FOREACH(svc->StartAfter) {
            snprintf(tmp_buffer, UPK_MAX_STRING_LEN, "%ld", idx++);
            LIST_UPSERT_TEXT(StartAfter, name, tmp_buffer);
            LIST_UPSERT_TEXT(StartAfter, pkg, tmp_buffer);
        }
    }

    idx = 0;
    if(svc->RestartOnNamedPrereqRestart && svc->RestartOnNamedPrereqRestart->count > 0) {
        UPKLIST_FOREACH(svc->RestartOnNamedPrereqRestart) {
            snprintf(tmp_buffer, UPK_MAX_STRING_LEN, "%ld", idx++);
            LIST_UPSERT_TEXT(RestartOnNamedPrereqRestart, name, tmp_buffer);
            LIST_UPSERT_TEXT(RestartOnNamedPrereqRestart, pkg, tmp_buffer);
        }
    }

    idx = 0;
    if(svc->CustomActions && svc->CustomActions->count > 0) {
        snprintf(tmp_buffer, UPK_MAX_STRING_LEN, "%ld", idx++);
        UPKLIST_FOREACH(svc->CustomActions) {
            LIST_UPSERT_TEXT(CustomActions, name, tmp_buffer);
            LIST_UPSERT_TEXT(CustomActions, script, tmp_buffer);
        }
    }



    if(hash_buffer) 
        free(hash_buffer);
    return hash;
}

/** ******************************************************************************************************************
  @brief Insert a configuration into the database

  @param[in] dbh                    The database handle.
  @param[in] svc                    The service description to insert.

  @return true on success, false on failure.
 ******************************************************************************************************************* */
bool
upk_db_insert_cfg(sqlite3 * dbh, upk_svc_desc_t * svc)
{
    char                    svcid[UPK_MAX_STRING_LEN] = "", *p = NULL;
    char                    uuid[UPK_UUID_STRING_LEN + 1] = "";

    upk_concat_svcid(svcid, svc->Package, svc->Name);
    upk_uuid_to_string(uuid, &svc->UUID);

    if(strcmp(uuid, "00000000-0000-0000-0000-000000000000") != 0) {
        /* update existing unless the existing one has a different UUID */
        UPK_FUNC_ASSERT_MSG((strncmp(upk_db_svc_uuid_lookup(dbh, svc->Name, svc->Package), uuid, UPK_UUID_STRING_LEN) != 0), UPK_CONF_ERROR,
                            "Conflicting UUID in data store: %s for %s", uuid, svcid);
    } else if((p = upk_db_svc_uuid_lookup(dbh, svc->Name, svc->Package))) {
        /* update existing */
        upk_string_to_uuid(&svc->UUID, p);
        upk_debug1("Updating existing service %s\n", svcid);
    } else {
        /* add new */
        upk_debug1("Adding new service for %s\n", svcid);
        upk_db_add_new_service(dbh, svc);
    }
    upk_db_upsert_options(dbh, svc);

    IF_UPK_ERROR {
    }

    return false;
}

/**
  @}
  */
