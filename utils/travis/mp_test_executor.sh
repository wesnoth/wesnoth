#!/bin/bash
set -e #Error if any line errors
set -m #Enable job control
set -v #Print shell commands as they are read

TIMEOUT_TIME=300
LOOP_TIME=6
HOSTLOG=/tmp/host.log
JOINLOG=/tmp/join.log

./wesnothd --port 12345 --log-debug=server --log-warning=config &
serverpid=$!

./wesnoth --plugin=host.lua --server=localhost:12345 --username=host --mp-test --noaddons --nogui 2> >(tee $HOSTLOG >&2) &
hostpid=$!

./wesnoth --plugin=join.lua --server=localhost:12345 --username=join --mp-test --noaddons --nogui 2> >(tee $JOINLOG >&2) &
joinpid=$!

START_TIME=$SECONDS
HOST_RUNNING=0
JOIN_RUNNING=0
while true; do
    # Timeout
    EXEC_TIME=$(($SECONDS - $START_TIME))
    if [ $EXEC_TIME -gt $TIMEOUT_TIME ]; then
        kill $hostpid 2>/dev/null
        kill $joinpid 2>/dev/null
    fi
    # Check if clients still running
    if ! kill -0 $hostpid 2>/dev/null; then
        HOST_RUNNING=1
    fi
    if ! kill -0 $joinpid 2>/dev/null; then
        JOIN_RUNNING=1
    fi

    sleep $LOOP_TIME

    # If both are finished, we're done
    if ! (kill -0 $hostpid 2>/dev/null || kill -0 $joinpid 2>/dev/null); then
        break
    fi
    # If one has finished previously, kill the other
    if [ $HOST_RUNNING == 1 ]; then
        echo "Host finished at least $LOOP_TIME seconds ago. Killing join"
        kill $joinpid 2>/dev/null
        break
    fi
    if [ $JOIN_RUNNING == 1 ]; then
        echo "Join finished at least $LOOP_TIME seconds ago. Killing host"
        kill $hostpid 2>/dev/null
        break
    fi
done

STATUS=0

wait $hostpid || (
    grep "Could not initialize SDL_video" $HOSTLOG && \
    echo -e "\nFailed to initialize video.\nThis is a common CI issue.\nTreating the test as successful.\n"
) || STATUS=1

wait $joinpid || (
    grep "Could not initialize SDL_video" $JOINLOG && \
    echo -e "\nFailed to initialize video.\nThis is a common CI issue.\nTreating the test as successful.\n"
) || STATUS=1

kill $serverpid

exit $STATUS
