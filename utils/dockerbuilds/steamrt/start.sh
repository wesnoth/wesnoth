#!/bin/sh
gamedir=${0%/*}
cd "$gamedir" || exit
LD_LIBRARY_PATH="$gamedir/lib${LD_LIBRARY_PATH:+":$LD_LIBRARY_PATH"}" exec ./wesnoth "$@"
