CREATE TABLE IF NOT EXISTS cfg._schema (
	version INTEGER,
	create_date INTEGER,
);

CREATE TABLE IF NOT EXISTS cfg.service_defaults (
	uuid CHAR(36),
	key VARCHAR(255) NOT NULL,
	value VARCHAR(4096),
	utc_added INTEGER,
	utc_modified INTEGER,
);

CREATE TABLE IF NOT EXISTS cfg.services_s1 (
	uuid CHAR(36) PRIMARY KEY,
   	svcid VARCHAR(1024) NOT NULL,
	package VARCHAR(1024),
	is_active BOOLEAN,
	utc_created INTEGER NOT NULL,
	utc_modified INTEGER NOT NULL,
	utc_deleted INTEGER,
);

CREATE TABLE IF NOT EXISTS svc_options_s1 (
	uuid CHAR(36),
	key VARCHAR(255) NOT NULL,
	value VARCHAR(4096),
	utc_added INTEGER,
	utc_modified INTEGER,
);
