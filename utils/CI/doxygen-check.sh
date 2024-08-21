#!/bin/bash

doxygen Doxyfile > doxy-output.txt 2>&1
LINES=`cat doxy-output.txt | wc -l`

if [ "$LINES" == "0" ]; then
    echo "No doxygen errors or warnings!"
else
    echo "Found doxygen errors or warnings!"
    cat doxy-output.txt
    exit 1
fi
