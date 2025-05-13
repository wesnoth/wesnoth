#!/bin/sh
gamedir=${0%/*}
cd "$gamedir" || exit
LD_LIBRARY_PATH=$gamedir/lib exec ./wesnoth "$@"
