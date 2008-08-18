#!/bin/bash
#/*
#   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
#   Part of the Battle for Wesnoth Project http://www.wesnoth.org/
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License version 2
#   or at your option any later version.
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY.
#
#   See the COPYING file for more details.
#*/

FULL_PATH=`dirname $(readlink -f $0)`
#source $FULL_PATH/path_settup.sh

export SSH_AUTH_SOCK=`find /tmp/keyring* -name ssh`
export DISPLAY=:0.0

WEBDIR=$FULL_PATH/../htdocs

cd $FULL_PATH/../trunk
nice php -f ../autotester/run_unit_tests.php $WEBDIR
#> $FULL_PATH/err.log
