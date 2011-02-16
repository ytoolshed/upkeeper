#!/bin/sh -x
DIR=`dirname $(which $0)`
DIR=$( (cd $DIR ; pwd -P) )
PATH=$PATH:$DIR:$DIR/../deps/sqlite
(cd $DIR && mkdir -p bt || echo ok)
cd $DIR/bt && echo "***" `pwd` && rm *sql; for i in ../../store/schema*sql; do ln -s $i .; done; ../../store/schema-setup.sh && exec ../buddytest
