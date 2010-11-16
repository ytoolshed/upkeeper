#!/bin/sh
exec 2>&1
echo "$$: Hello World!"  && exec sleep 10
