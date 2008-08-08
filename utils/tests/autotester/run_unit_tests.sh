#!/bin/sh

SVNDIR="/home/coren/wesnoth/trunk/"
WEBDIR="/home/coren/wesnoth/trunk/utils/tests/htdocs/"
AUTOTESTDIR="/home/coren/wesnoth/trunk/utils/tests/autotester/"
cd $SVNDIR
nice php -f $AUTOTESTDIR/run_unit_tests.php $WEBDIR
