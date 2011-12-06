-- The schema table is used to lookup table names. All changes to this file
-- should be additive; never remove prior sql, only add to it, translate
-- prior versions to new versions and remove old versions in sql; not by
-- removing them from this file. Clean and lossless conversion from one
-- schema version to the next must occur as a result of running this sql.
CREATE TABLE IF NOT EXISTS upk_cfg._schema (
	version INTEGER NOT NULL,
	create_date INTEGER NOT NULL,
	table_name varchar(256) PRIMARY KEY,
	table_id varchar(256) NOT NULL
);
CREATE TABLE IF NOT EXISTS upk_cfg.services_s1 (
	uuid CHAR(37) PRIMARY KEY,
   	svcid VARCHAR(2048) NOT NULL,
	package VARCHAR(2048),
	is_active BOOLEAN,
	utc_created INTEGER NOT NULL,
	utc_modified INTEGER NOT NULL,
	utc_deleted INTEGER
);
DELETE FROM upk_cfg._schema WHERE table_name = "services";
INSERT INTO upk_cfg._schema (version, create_date, table_name, table_id) VALUES (1, 1312914180, "services", "services_s1");
CREATE TABLE IF NOT EXISTS upk_cfg.svc_options_s1 (
	uuid CHAR(37) NOT NULL,
	key VARCHAR(256) NOT NULL,
	value VARCHAR(4096),
	utc_added INTEGER,
	utc_modified INTEGER,
	PRIMARY KEY(uuid, key)
);
DELETE FROM upk_cfg._schema WHERE table_name = "svc_options";
INSERT INTO upk_cfg._schema (version, create_date, table_name, table_id) VALUES (1, 1312914180, "svc_options", "svc_options_s1");
CREATE TABLE IF NOT EXISTS upk_cfg.svc_list_options_s1 (
	uuid CHAR(37) NOT NULL,
	section VARCHAR(256),
	key VARCHAR(256) NOT NULL,
	id VARCHAR(256),
	value VARCHAR(256) NOT NULL,
	utc_added INTEGER,
	utc_modified INTEGER
);
DELETE FROM upk_cfg._schema WHERE table_name = "svc_list_options";
INSERT INTO upk_cfg._schema (version, create_date, table_name, table_id) VALUES (1, 1312914180, "svc_list_options", "svc_list_options_s1");
CREATE TABLE IF NOT EXISTS upk_cfg.svc_option_hashes_s1 (
	uuid CHAR(37) NOT NULL PRIMARY KEY,
	options_crc32 INTEGER NOT NULL,
	list_options_crc32 INTEGER NOT NULL
);
DELETE FROM upk_cfg._schema WHERE table_name = "svc_option_hashes";
INSERT INTO upk_cfg._schema (version, create_date, table_name, table_id) VALUES (1, 1312914180, "svc_option_hashes", "svc_option_hashes_s1");

