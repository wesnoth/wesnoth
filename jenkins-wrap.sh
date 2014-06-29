#!/bin/sh
COUNTER=9

export PATH="/usr/local/bin:$PATH"

run_test() {
    xvfb-run -e xvfb-err "$@"
}

run_test "$@"
ERRORCODE=$?
while [ $COUNTER -gt 0 -a $ERRORCODE -eq 1 ];do
    if [ -s xvfb-err ]; then
        echo "xvfb output:"
        cat xvfb-err
        rm xvfb-err
    fi
    echo "\n\'xvfb-run $@' returned 1, which could mean that xvfb failed. Retrying..."
    run_test "$@"
    ERRORCODE=$?
    COUNTER=$(($COUNTER - 1))
done

rm -f xvfb-err
exit $ERRORCODE
