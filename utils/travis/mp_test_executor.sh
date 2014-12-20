#!/bin/bash
set -e #Error if any line errors
set -m #Enable job control
set -v #Print shell commands as they are read

./wesnothd --port 12345 --log-debug=server --log-warning=config &
serverpid=$!

./wesnoth --plugin=host.lua --server=localhost:12345 --username=host --nogui --mp-test &
hostpid=$!

./wesnoth --plugin=join.lua --server=localhost:12345 --username=join --nogui --mp-test &
joinpid=$!

wait $hostpid

wait $joinpid

kill $serverpid

exit 0
