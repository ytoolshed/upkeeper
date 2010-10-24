DROP TABLE IF EXISTS events;
DROP TABLE IF EXISTS services;
DROP TABLE IF EXISTS procruns;




CREATE TABLE procruns (
    id            INTEGER PRIMARY KEY,
    cmdline       VARCHAR,
    user          VARCHAR,
    pid           INTEGER
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
    event      VARCHAR,
    service_id INT,
    FOREIGN KEY (service_id)
        REFERENCES service(id)
);

CREATE TRIGGER signal_buddy UPDATE OF state_desired ON services
WHEN   new.state_desired = 'stop'
       BEGIN
              SELECT     signal_send(procruns.pid,15)
              FROM       procruns 
              WHERE      new.procrun_id    = procruns.id;
       END
 ;

