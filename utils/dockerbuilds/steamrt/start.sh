#!/bin/sh
gamedir=${0%/*}
cd "$gamedir" || exit
exec ./wesnoth "$@"
