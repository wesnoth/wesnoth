#!/bin/bash
set -e #Error if any line errors
set -m #Enable job control
set -v #Print shell commands as they are read

TIMEOUT_TIME=300
LOOP_TIME=6

./wesnothd --port 12345 --log-debug=server --log-warning=config &> wesnothd.log &
serverpid=$!
sleep 5

./wesnoth --plugin=data/test/plugin/host.lua --server=localhost:12345 --username=host --mp-test --noaddons --nogui &> wesnoth-host.log &
hostpid=$!
sleep 2

while grep -q 'Could not initialize SDL_video' wesnoth-host.log; do
    echo "Could not initialize SDL_video error, retrying..."
    ./wesnoth --plugin=data/test/plugin/host.lua --server=localhost:12345 --username=host --mp-test --noaddons --nogui &> wesnoth-host.log &
    hostpid=$!
    sleep 2
done

./wesnoth --plugin=data/test/plugin/join.lua --server=localhost:12345 --username=join --mp-test --noaddons --nogui &> wesnoth-join.log &
joinpid=$!
sleep 2

while grep -q 'Could not initialize SDL_video' wesnoth-join.log; do
    echo "Could not initialize SDL_video error, retrying..."
    ./wesnoth --plugin=data/test/plugin/join.lua --server=localhost:12345 --username=join --mp-test --noaddons --nogui &> wesnoth-join.log &
    joinpid=$!
    sleep 2
done

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

echo "Server log:"
cat wesnothd.log

echo "Host log:"
cat wesnoth-host.log

echo "Join log:"
cat wesnoth-join.log

exit $STATUS
