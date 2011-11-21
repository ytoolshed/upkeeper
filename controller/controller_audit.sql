CREATE DATABASE upk_audit;

CREATE TABLE IF NOT EXISTS upk_audit.service_s1 (
	uuid CHAR(36),
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
	timestamp INTEGER,
);
