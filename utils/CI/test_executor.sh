#!/bin/bash
gdb -q -batch -return-child-result -ex "set disable-randomization off" -ex "set style enabled on" -ex "run" -ex "thread apply all bt" -ex "quit" --args ./boost_unit_tests --color_output --log_level=test_suite
error_code="$?"
while grep -q 'Could not initialize SDL_video' error.log; do
  echo "Could not initialize SDL_video error, retrying..."
  gdb -q -batch -return-child-result -ex "set style enabled on" -ex "run" -ex "thread apply all bt" -ex "quit" --args ./boost_unit_tests --color_output --log_level=test_suite
  error_code="$?"
done

exit $error_code

