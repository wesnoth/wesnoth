#!/bin/bash
set -e
gdb -q -batch -return-child-result -ex "run" -ex "thread apply all bt" -ex "quit" --args ./wesnoth -m --controller 1:ai --controller 2:ai
