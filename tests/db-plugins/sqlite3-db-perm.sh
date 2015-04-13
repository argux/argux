TEMPDB=`mktemp /tmp/argux-tst-db.XXXXXX`
TEMPCONF=`mktemp /tmp/argux-tst-conf.XXXXXX`


# Create Database
cat $srcdir/../../data/schema/sqlite3.schema |\
    sqlite3 $TEMPDB

# Create Config File
echo "db_path=$TEMPDB" >> $TEMPCONF

./plugin-loader --db-perm \
                --config=$TEMPCONF \
                sqlite3-db-plugin

RET=$?

unlink $TEMPDB
unlink $TEMPCONF

exit $RET
