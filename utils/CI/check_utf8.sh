#!/bin/bash
# Install isutf8 program (from package "moreutils" at least in linux mint) in order to use this script.
#
# This script assumes that the current working directory is the root of the wesnoth repository.

command -v isutf8 >/dev/null || { echo "Install 'isutf8' from moreutils to use this script."; exit 1; }

exit_code=0

find src/ -path src/modules -prune -o -type f -print0 | xargs -0 isutf8 -- || exit_code=1
for ex in png ogg jpg wav gif xcf bin webp; do args+=(! -name "*.$ex"); done
find data/ -type f "${args[@]}" ! -name "test_cve_2018_1999023_2.cfg" -print0 | xargs -0 isutf8 -- || exit_code=1
find po/ -type f -print0 | xargs -0 isutf8 -- || exit_code=1

isutf8 changelog.md || exit_code=1

if [ "$exit_code" != 0 ]; then
    echo "Found invalid UTF8 file(s)!"
fi

exit "$exit_code"
