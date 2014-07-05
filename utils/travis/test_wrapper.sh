#!/bin/bash
COUNTER=10
./test
ERRORCODE=$?
while [ $COUNTER -gt 0 -a $ERRORCODE -eq 200 ]; do
    echo "test apparently timed out with error code 200... trying again."
    COUNTER=$((COUNTER-1))
    ./test
    ERRORCODE=$?
done
exit $ERRORCODE
