#!/bin/bash
set -e #Error if any line errors
set -m #Enable job control
set -v #Print shell commands as they are read

./wesnothd --port 12345 --log-debug=server --log-warning=config &
serverpid=$!

./wesnoth --plugin=host-gui2.lua --server=localhost:12345 --username=host --mp-test --noaddons --nogui &
hostpid=$!

./wesnoth --plugin=join-gui2.lua --server=localhost:12345 --username=join --mp-test --noaddons --nogui &
joinpid=$!

wait $hostpid

wait $joinpid

kill $serverpid

exit 0
