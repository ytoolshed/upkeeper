#!/bin/sh
PATH=$PATH:`dirname $0`/../deps/sqlite
cd `dirname $0` && ./init.sh && exec ./store-test
