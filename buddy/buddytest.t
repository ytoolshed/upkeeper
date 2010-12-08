#!/bin/sh 
cd `dirname $0`
(mkdir -p bt || echo ok)
(ln -f buddy bt/)
(cd bt && ../buddytest)
