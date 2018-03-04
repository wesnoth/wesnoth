#!/bin/bash
set -e #Error if any line errors
set -m #Enable job control
set -v #Print shell commands as they are read

TIMEOUT_TIME=300
LOOP_TIME=6

./wesnothd --port 12345 --log-debug=server --log-warning=config &
serverpid=$!

sleep 5

./wesnoth --plugin=host.lua --server=localhost:12345 --username=host --mp-test --noaddons --nogui &
hostpid=$!

sleep 5

./wesnoth --plugin=join.lua --server=localhost:12345 --username=join --mp-test --noaddons --nogui &
joinpid=$!

START_TIME=$SECONDS
HOST_RUNNING=yes
JOIN_RUNNING=yes
while true; do
    # Timeout
    EXEC_TIME=$(($SECONDS - $START_TIME))
    if [ $EXEC_TIME -gt $TIMEOUT_TIME ]; then
        kill $hostpid 2>/dev/null
        kill $joinpid 2>/dev/null
        break
    fi
    # Check if clients still running
    if ! kill -0 $hostpid 2>/dev/null; then
        HOST_RUNNING=no
    fi
    if ! kill -0 $joinpid 2>/dev/null; then
        JOIN_RUNNING=no
    fi

    sleep $LOOP_TIME

    # If both are finished, we're done
    if ! (kill -0 $hostpid 2>/dev/null || kill -0 $joinpid 2>/dev/null); then
        break
    fi
    # If one has finished previously, kill the other
    if [ $HOST_RUNNING = "no" ]; then
        echo "Host finished at least $LOOP_TIME seconds ago. Killing join"
        kill $joinpid 2>/dev/null
        break
    fi
    if [ $JOIN_RUNNING = "no" ]; then
        echo "Join finished at least $LOOP_TIME seconds ago. Killing host"
        kill $hostpid 2>/dev/null
        break
    fi
done

STATUS=0

wait $hostpid || STATUS=1

wait $joinpid || STATUS=1

kill $serverpid

exit $STATUS
