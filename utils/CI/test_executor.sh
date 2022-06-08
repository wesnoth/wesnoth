#!/bin/bash
gdb -q -batch -return-child-result -ex "set disable-randomization off" -ex "set style enabled on" -ex "run" -ex "thread apply all bt" -ex "quit" --args ./boost_unit_tests 2> error.log
error_code="$?"
while grep -q 'Could not initialize SDL_video' error.log; do
  echo "Could not initialize SDL_video error, retrying..."
  gdb -q -batch -return-child-result -ex "set style enabled on" -ex "run" -ex "thread apply all bt" -ex "quit" --args ./boost_unit_tests 2> error.log
  error_code="$?"
done
cat error.log

exit $error_code

