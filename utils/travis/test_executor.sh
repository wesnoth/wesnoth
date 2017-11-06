#!/bin/bash
set -e
gdb -q -batch -return-child-result -ex "run" -ex "thread apply all bt" -ex "quit" --args ./boost_unit_tests
