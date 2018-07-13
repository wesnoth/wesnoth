#!/bin/bash
# Install isutf8 program (from package "moreutils" at least in linux mint) in order to use this script.
#
# This script assumes that the current working directory is the root of the wesnoth repository.
exit_code=0

find src/ -type f -print0 | xargs -0 isutf8 -- || exit_code=1
find data/ -not -name "*.png" -not -name "*.ogg" -not -name "*.jpg" -not -name "*.wav" -not -name "*.gif" -not -name "*.xcf" -type f -print0 | xargs -0 isutf8 -- || exit_code=1
find po/ -type f -print0 | xargs -0 isutf8 -- || exit_code=1

isutf8 changelog.md || exit_code=1
isutf8 RELEASE_NOTES || exit_code=1

if [ $exit_code != 0 ]; then
    echo "Found invalid UTF8 file(s)!"
fi

exit $exit_code
