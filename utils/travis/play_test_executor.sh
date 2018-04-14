#!/bin/bash
gdb -q -batch -return-child-result -ex "run" -ex "thread apply all bt" -ex "quit" --args ./wesnoth -m --controller 1:ai --controller 2:ai --nogui 2> error.log
error_code="$?"
while grep -q 'Could not initialize SDL_video' error.log; do
  echo "Could not initialize SDL_video error, retrying..."
  gdb -q -batch -return-child-result -ex "run" -ex "thread apply all bt" -ex "quit" --args ./wesnoth -m --controller 1:ai --controller 2:ai --nogui 2> error.log
  error_code="$?"
done
cat error.log

exit $error_code

