
DROP TABLE IF EXISTS listeners;

CREATE TABLE listeners (
    id        INTEGER PRIMARY KEY,
    component VARCHAR,
    signal    INTEGER,
    pid       INTEGER
);
