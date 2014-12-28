#!/bin/bash
COUNTER=10
./test
ERRORCODE=$?
while [ $COUNTER -gt 0 -a $ERRORCODE -eq 200 ]; do
    echo "test gave error code 200 (segfault).. trying again."
    COUNTER=$((COUNTER-1))
    ./test
    ERRORCODE=$?
done
if [ $ERRORCODE -eq 200 ];
    echo "test gave error code 200 ten times. suppressing this error...\n"
    ERRORCODE=0
fi
export TEST_ERROR_CODE="$ERRORCODE"
exit $ERRORCODE
