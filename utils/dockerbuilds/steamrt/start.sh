#!/bin/sh
gamedir=${0%/*}
cd "$gamedir" || exit
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$gamedir/lib64"
exec ./wesnoth "$@"
