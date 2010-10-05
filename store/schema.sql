DROP TABLE IF EXISTS events;
DROP TABLE IF EXISTS services;
DROP TABLE IF EXISTS processes;

CREATE TABLE processes (
    id            INTEGER PRIMARY KEY,
    cmdline       VARCHAR,
    user          VARCHAR
);

CREATE TABLE services (
    id            INTEGER PRIMARY KEY,
    package       VARCHAR,
    service       VARCHAR,
    process_id    INT,
    state_desired INT,
    state_actual  INT,
    FOREIGN KEY (process_id)
        REFERENCES processes(id)
);

CREATE TABLE events (
    id         INTEGER PRIMARY KEY,
    etime      DATETIME,
    event      VARCHAR,
    service_id INT,
    FOREIGN KEY (service_id)
        REFERENCES service(id)
);
