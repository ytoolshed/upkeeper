DROP TABLE IF EXISTS events;
DROP TABLE IF EXISTS services;
DROP TABLE IF EXISTS procruns;
DROP TABLE IF EXISTS listeners;




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
    state_desired VARCHAR,
    state_actual  VARCHAR,
    FOREIGN KEY (procrun_id)
        REFERENCES procrun(id)
);

CREATE TABLE events (
    id         INTEGER PRIMARY KEY,
    etime      DATETIME,
    event      VARCHAR(64),
    service_id INT,
    pid        INT,
    FOREIGN KEY (service_id)
        REFERENCES service(id)
);

CREATE TABLE listeners (
    id        INTEGER PRIMARY KEY,
    component VARCHAR,
    signal    INTEGER,
    pid       INTEGER
);

CREATE TRIGGER add_pid AFTER INSERT on events
BEGIN
        UPDATE events SET pid = get_pid() WHERE id = new.id;
END;

CREATE TRIGGER buddy_pid AFTER UPDATE OF pid on procruns
BEGIN
        UPDATE procruns SET bpid = get_pid() WHERE id = new.id;
END;

CREATE TRIGGER signal_buddy UPDATE OF state_desired ON services
WHEN   new.state_desired = 'stop'
       BEGIN
              INSERT INTO events
              (etime,event,service_id)
              SELECT     datetime('now'),
                         'kill' + procruns.pid + signal_send(procruns.bpid,15),
                         new.id
              FROM  procruns 
              WHERE new.procrun_id    = procruns.id
              and   procruns.pid     is not null
              and   procruns.pid     != 0;
END;