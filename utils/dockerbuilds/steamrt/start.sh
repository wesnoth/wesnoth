#!/bin/bash
GAMEDIR=$(cd "${0%/*}" && echo $PWD)
cd "$GAMEDIR"
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$GAMEDIR/lib64"
exec ./wesnoth "$@"
