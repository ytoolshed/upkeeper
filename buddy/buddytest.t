#!/bin/sh -xe
cd `dirname $0`
mkdir bt || echo ok
(cd bt && cat ../../store/schema.sql | sqlite3 store.sqlite) 
(cd bt && ../buddytest)
(cd bt && ../../store/sqlite-dump)