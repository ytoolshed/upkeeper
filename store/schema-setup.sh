
rm -f upkeeper-main.sqlite upkeeper-misc.sqlite

sqlite3 upkeeper-main.sqlite <schema-main.sql
sqlite3 upkeeper-misc.sqlite <schema-misc.sql
