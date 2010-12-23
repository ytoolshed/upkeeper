#!/bin/sh 
DIR=`dirname $(which $0)`
DIR=$( (cd $DIR ; pwd -P) )
PATH=$PATH:$DIR
(cd $DIR && mkdir -p bt || echo ok)
cd $DIR/bt && rm -f store.sqlite && sqlite3 store.sqlite < ../../store/schema.sql && exec ../buddytest
