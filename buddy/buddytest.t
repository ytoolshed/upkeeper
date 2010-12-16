#!/bin/sh 
cd `dirname $0`
(mkdir -p bt || echo ok)
(ln -sf ../buddy bt/)
cd bt && rm -f store.sqlite && sqlite3 store.sqlite < ../../store/schema.sql && exec ../buddytest
