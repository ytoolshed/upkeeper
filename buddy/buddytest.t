#!/bin/sh 
cd `dirname $0`
(mkdir -p bt || echo ok)
(cd bt && ../buddytest)
