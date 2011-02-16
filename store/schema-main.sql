DROP TABLE IF EXISTS events;
DROP TABLE IF EXISTS exits;
DROP TABLE IF EXISTS services;
DROP TABLE IF EXISTS procruns;
DROP TABLE IF EXISTS namevalue;

CREATE TABLE namevalue (
    id            INTEGER PRIMARY KEY,
    name          VARCHAR,
    value         VARCHAR
);

CREATE TABLE procruns (
    id            INTEGER PRIMARY KEY,
    cmdline       VARCHAR,
    user          VARCHAR,
    pid           INTEGER,
    bpid          INTEGER
);

CREATE TABLE services (
    id            INTEGER PRIMARY KEY,
    package       VARCHAR,
    service       VARCHAR,
    procrun_id    INT,
    state_desired VARCHAR default 'stop',
    state_actual  VARCHAR default 'stop',
    FOREIGN KEY (procrun_id)
        REFERENCES procrun(id)
);
CREATE UNIQUE index buddy on procruns (bpid);
CREATE UNIQUE index service on services (package,service);

CREATE TABLE events (
    id         INTEGER PRIMARY KEY,
    etime      DATETIME,
    event      VARCHAR(64),
    service_id INT,
    pid        INT,
    FOREIGN KEY (service_id)
        REFERENCES service(id)
);

CREATE TABLE exits (
    id         INTEGER PRIMARY KEY,
    procrun_id INT,
    status     INT,
    FOREIGN KEY (procrun_id)
        REFERENCES procrun(id)
);

INSERT INTO namevalue (name, value) VALUES ('created', datetime('now'));
