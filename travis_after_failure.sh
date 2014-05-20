#!/bin/bash
if [ -f "errors.log" ]; then
  echo -e "\n*** \n*"
  echo "* Errors reported in wml unit tests, here is errors.log..."
  echo -e "*\n*** \n"
  cat errors.log
fi
