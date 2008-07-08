#! /bin/sh

set -e

TESTING_DATABASE=/tmp/wesnoth-uploads.db
TESTING_LOGFILE=/tmp/wesnoth-upload-log.
export TESTING_DATABASE
export TESTING_LOGFILE

for f in testsuite/*.input; do
    echo $f
    rm -f $TESTING_DATABASE $TESTING_LOGFILE*
    ./upload.cgi --initialize
    echo 'INSERT INTO bad_serial VALUES ("BADSERIAL");' | sqlite3 $TESTING_DATABASE
    for i in $f*; do
	case $i in *~) :;; *)
		if ./upload.cgi < $i > /dev/null; then :
		else echo ERROR:; cat ${TESTING_LOGFILE}0; exit 1
		fi;;
	esac
    done
    BASE=`echo $f | sed 's/\.input$//'`
    [ x"`sqlite3 $TESTING_DATABASE < $BASE.test`" = x"`cat $BASE.output`" ]
done

echo -n Parallel test
rm -f $TESTING_DATABASE $TESTING_LOGFILE*
./upload.cgi --initialize

parallel_test()
{
    while [ -f $1 ]; do sleep 0; done
    if ./upload.cgi < $2 > /dev/null; then
	echo $2 succeeded >> $3
	echo -n .
    else
	echo $2 failed >> $3
    fi
}

STARTFILE=`mktemp`
OUTPUTFILE=`mktemp`
trap "rm -f $OUTPUTFILE $STARTFILE" EXIT

COUNT=0
for i in testsuite/*.parallel; do
    COUNT=$(($COUNT + 1))
    parallel_test $STARTFILE $i $OUTPUTFILE &
done

rm $STARTFILE
while [ `wc -l < $OUTPUTFILE` -lt $COUNT ]; do
    sleep 1
done
echo

if grep failed $OUTPUTFILE; then
    exit 1
fi

[ x"`sqlite3 $TESTING_DATABASE < testsuite/parallel-test`" = x"`cat testsuite/parallel-output`" ]
echo Succeeded.
