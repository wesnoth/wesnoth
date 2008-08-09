#!/bin/bash
FULL_PATH=`dirname $(readlink -f $0)`
source $FULL_PATH/path_settup.sh

export SSH_AUTH_SOCK=`find /tmp/keyring* -name ssh`
export DISPLAY=:0.0

cd $SVNDIR
nice php -f ${AUTOTESTDIR}/run_unit_tests.php $WEBDIR > $FULL_PATH/err.log
