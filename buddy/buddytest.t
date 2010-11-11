#!/bin/sh -xe

mkdir bt || echo ok
(cd bt && cat ../../store/schema.sql | sqlite3 store.sqlite) 
(cd bt && ../buddytest)
(cd bt && ../../store/sqlite-dump)