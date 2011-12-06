-- The schema table is used to lookup table names. All changes to this file
-- should be additive; never remove prior sql, only add to it, translate
-- prior versions to new versions and remove old versions in sql; not by
-- removing them from this file. Clean and lossless conversion from one
-- schema version to the next must occur as a result of running this sql.
CREATE TABLE IF NOT EXISTS upk_audit._schema (
	version INTEGER NOT NULL,
	create_date INTEGER NOT NULL,
	table_name varchar(256) PRIMARY KEY,
	table_id varchar(256) NOT NULL
);
CREATE TABLE IF NOT EXISTS upk_audit.service_s1 (
	uuid CHAR(36),
	svcid VARCHAR(2048) NOT NULL,
	package VARCHAR(2048),
	service_pid INTEGER,
	wait_pid INTEGER,
	command VARCHAR(255) NOT NULL,
	command_n INTEGER NOT NULL,
	si_signo INTEGER,
	si_errno INTEGER,
	si_code INTEGER,
	si_pid INTEGER,
	si_uid INTEGER,
	si_status INTEGER,
	wait_status INTEGER,
	timestamp INTEGER
);
-- updating the schema table for this version's service audit table.
DELETE FROM upk_audit._schema WHERE table_name = "service";
INSERT INTO upk_audit._schema (version, create_date, table_name, table_id) VALUES (1, 1312914180, "service", "service_s1");
