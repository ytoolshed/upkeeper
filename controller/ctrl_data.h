
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

#ifndef _CTRL_DATA_H
#define _CTRL_DATA_H

/* ctrl_data.c */
extern int upk_db_get_single_text(sqlite3 *dbh, char *value, const char *sql, ...);
extern int upk_db_get_audit_service_table(sqlite3 *dbh, char *table_id);
extern int upk_db_get_cfg_services_table(sqlite3 *dbh, char *table_id);
extern int upk_db_get_cfg_svc_options_table(sqlite3 *dbh, char *table_id);
extern char *upk_db_svc_uuid_lookup(sqlite3 *dbh, const char *svcid, const char *package);
extern _Bool upk_db_insert_cfg(sqlite3 *dbh, upk_svc_desc_t *svc);

#endif
