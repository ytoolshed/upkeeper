#!/bin/sh -x
DIR=`dirname $(which $0)`
DIR=$( (cd $DIR ; pwd -P) )
PATH=$PATH:$DIR:$DIR/../deps/sqlite
(cd $DIR && mkdir -p bt || echo ok)
cd $DIR/bt && rm -f upkeeper.sqlite && sqlite3 upkeeper.sqlite < ../../store/schema.sql && exec ../buddytest
