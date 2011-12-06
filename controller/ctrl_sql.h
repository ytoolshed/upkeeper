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

#ifndef _UPK_SQL_H
#define _UPK_SQL_H

/* *INDENT-OFF* */
const char ctrl_create_audit_sql[] = 
		"-- The schema table is used to lookup table names. All changes to this file\n"
		"-- should be additive; never remove prior sql, only add to it, translate\n"
		"-- prior versions to new versions and remove old versions in sql; not by\n"
		"-- removing them from this file. Clean and lossless conversion from one\n"
		"-- schema version to the next must occur as a result of running this sql.\n"
		"CREATE TABLE IF NOT EXISTS upk_audit._schema (\n"
		"	version INTEGER NOT NULL,\n"
		"	create_date INTEGER NOT NULL,\n"
		"	table_name varchar(256) PRIMARY KEY,\n"
		"	table_id varchar(256) NOT NULL\n"
		");\n"
		"CREATE TABLE IF NOT EXISTS upk_audit.service_s1 (\n"
		"	uuid CHAR(36),\n"
		"	svcid VARCHAR(2048) NOT NULL,\n"
		"	package VARCHAR(2048),\n"
		"	service_pid INTEGER,\n"
		"	wait_pid INTEGER,\n"
		"	command VARCHAR(255) NOT NULL,\n"
		"	command_n INTEGER NOT NULL,\n"
		"	si_signo INTEGER,\n"
		"	si_errno INTEGER,\n"
		"	si_code INTEGER,\n"
		"	si_pid INTEGER,\n"
		"	si_uid INTEGER,\n"
		"	si_status INTEGER,\n"
		"	wait_status INTEGER,\n"
		"	timestamp INTEGER\n"
		");\n"
		"-- updating the schema table for this version's service audit table.\n"
		"DELETE FROM upk_audit._schema WHERE table_name = \"service\";\n"
		"INSERT INTO upk_audit._schema (version, create_date, table_name, table_id) VALUES (1, 1312914180, \"service\", \"service_s1\");\n"
;
const char ctrl_create_cfg_sql[] = 
		"-- The schema table is used to lookup table names. All changes to this file\n"
		"-- should be additive; never remove prior sql, only add to it, translate\n"
		"-- prior versions to new versions and remove old versions in sql; not by\n"
		"-- removing them from this file. Clean and lossless conversion from one\n"
		"-- schema version to the next must occur as a result of running this sql.\n"
		"CREATE TABLE IF NOT EXISTS upk_cfg._schema (\n"
		"	version INTEGER NOT NULL,\n"
		"	create_date INTEGER NOT NULL,\n"
		"	table_name varchar(256) PRIMARY KEY,\n"
		"	table_id varchar(256) NOT NULL\n"
		");\n"
		"CREATE TABLE IF NOT EXISTS upk_cfg.services_s1 (\n"
		"	uuid CHAR(37) PRIMARY KEY,\n"
		"   	svcid VARCHAR(2048) NOT NULL,\n"
		"	package VARCHAR(2048),\n"
		"	is_active BOOLEAN,\n"
		"	utc_created INTEGER NOT NULL,\n"
		"	utc_modified INTEGER NOT NULL,\n"
		"	utc_deleted INTEGER\n"
		");\n"
		"DELETE FROM upk_cfg._schema WHERE table_name = \"services\";\n"
		"INSERT INTO upk_cfg._schema (version, create_date, table_name, table_id) VALUES (1, 1312914180, \"services\", \"services_s1\");\n"
		"CREATE TABLE IF NOT EXISTS upk_cfg.svc_options_s1 (\n"
		"	uuid CHAR(37) NOT NULL,\n"
		"	key VARCHAR(256) NOT NULL,\n"
		"	value VARCHAR(4096),\n"
		"	utc_added INTEGER,\n"
		"	utc_modified INTEGER,\n"
		"	PRIMARY KEY(uuid, key)\n"
		");\n"
		"DELETE FROM upk_cfg._schema WHERE table_name = \"svc_options\";\n"
		"INSERT INTO upk_cfg._schema (version, create_date, table_name, table_id) VALUES (1, 1312914180, \"svc_options\", \"svc_options_s1\");\n"
		"CREATE TABLE IF NOT EXISTS upk_cfg.svc_list_options_s1 (\n"
		"	uuid CHAR(37) NOT NULL,\n"
		"	section VARCHAR(256),\n"
		"	key VARCHAR(256) NOT NULL,\n"
		"	id VARCHAR(256),\n"
		"	value VARCHAR(256) NOT NULL,\n"
		"	utc_added INTEGER,\n"
		"	utc_modified INTEGER\n"
		");\n"
		"DELETE FROM upk_cfg._schema WHERE table_name = \"svc_list_options\";\n"
		"INSERT INTO upk_cfg._schema (version, create_date, table_name, table_id) VALUES (1, 1312914180, \"svc_list_options\", \"svc_list_options_s1\");\n"
		"CREATE TABLE IF NOT EXISTS upk_cfg.svc_option_hashes_s1 (\n"
		"	uuid CHAR(37) NOT NULL PRIMARY KEY,\n"
		"	options_crc32 INTEGER NOT NULL,\n"
		"	list_options_crc32 INTEGER NOT NULL\n"
		");\n"
		"DELETE FROM upk_cfg._schema WHERE table_name = \"svc_option_hashes\";\n"
		"INSERT INTO upk_cfg._schema (version, create_date, table_name, table_id) VALUES (1, 1312914180, \"svc_option_hashes\", \"svc_option_hashes_s1\");\n"
		"\n"
;
/* *INDENT-ON* */
#endif
