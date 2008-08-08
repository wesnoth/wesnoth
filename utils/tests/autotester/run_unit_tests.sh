#!/bin/bash
FULL_PATH=`dirname $(readlink -f $0)`
source $FULL_PATH/path_settup.sh

cd $SVNDIR
nice php -f ${AUTOTESTDIR}/run_unit_tests.php $WEBDIR
