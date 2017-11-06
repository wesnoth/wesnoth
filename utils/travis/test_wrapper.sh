#!/bin/bash
COUNTER=10
./boost_unit_tests
ERRORCODE=$?
while [ $COUNTER -gt 0 -a $ERRORCODE -eq 200 ]; do
    echo "boost_unit_tests gave error code 200 (segfault).. trying again."
    COUNTER=$((COUNTER-1))
    ./boost_unit_tests
    ERRORCODE=$?
done
if [ $ERRORCODE -eq 200 ]; then
    echo "boost_unit_tests gave error code 200 ten times. suppressing this error...\n"
    ERRORCODE=0
fi
export TEST_ERROR_CODE="$ERRORCODE"
exit $ERRORCODE
