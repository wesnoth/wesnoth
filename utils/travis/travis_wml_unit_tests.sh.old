#!/bin/bash
./run_wml_tests -u -w
if [ "$?" -ne 0 ]; then
  echo -e "\n*** \n*"
  echo "* Rerunning tests to get error messages.."
  echo -e "*\n*** \n"
  ./run_wml_tests -u -w -s
fi
