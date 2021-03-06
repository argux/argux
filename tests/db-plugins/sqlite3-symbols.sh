#/bin/sh

PLUGINDIR="../../plugins"
PLUGINSUBDIR=".libs"

env

TEMP=`mktemp /tmp/argux-tst.XXXXXX`

nm -g $PLUGINDIR/sqlite3-db-plugin/$PLUGINSUBDIR/sqlite3-db-plugin.so \
| awk '{ print $3 }' | sed '/^$/d' > $TEMP
    
diff -u $TEMP $srcdir/sqlite3-db-plugin.symbols
RET=$?

unlink $TEMP

exit $RET
