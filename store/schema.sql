DROP TABLE IF EXISTS events;
DROP TABLE IF EXISTS services;
DROP TABLE IF EXISTS processes;




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
    state_desired INT DEFAULT 'stop',
    state_actual  INT DEFAULT 'stop',
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
              INSERT INTO events
              (etime,event,service_id)
              SELECT     datetime('now'),
                         'kill' + procruns.pid + signal_send(procruns.pid,15),
                         new.id
              FROM  procruns 
              WHERE new.procrun_id    = procruns.id;
       END
 ;