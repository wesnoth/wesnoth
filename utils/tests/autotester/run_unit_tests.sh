#!/bin/bash

source path_settup.sh

cd $SVNDIR
nice php -f ${AUTOTESTDIR}/run_unit_tests.php $WEBDIR
