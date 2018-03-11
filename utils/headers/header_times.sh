#!/bin/bash
#Compute header times. Takes an scons build log with debug=time on, file name as first and only arg.
set -e

if [[ "$#" -ne 1 ]]; then
    echo "Usage:" $0 " [scons-log-file]"
    echo
    echo "Ranks headers according to the aggregate build time of compilation units which"
    echo "read them. In other words, the ranking answers the question 'if I stopped a"
    echo "wesnoth build at a random point in time, which headers are most likely to have"
    echo "been read by that compilation unit?'"
    echo
    echo "Expects to take the name of a log file from an scons build (with debug=time"
    echo "option passed in) as first and only arg. This file must be located at the root"
    echo "of the wesnoth repository directory, and the argument should just be its name"
    echo "and extension."
    echo
    echo "Expects the current working directory to be the root directory of the repo."
    echo
    echo "Example Usage:"
    echo
    echo -e "\t./build_headers.sh -s"
    echo -e "\t./header_times.sh travis_log_sample.log"
    echo
    exit 1;
fi

if [ -d headers-annotated ]; then
    rm -r headers-annotated
fi

cp -fR headers headers-annotated
cd headers-annotated/
for file in `find . -name "*.cpp" -type f -print0 | xargs -0`; do
    echo "src/${file:2}"
    if grep -q "src/${file:2}$" "../$1"; then
	#echo "match:" '\_ src/'"${file:2}$"'_ { N; s/.*\n//p; }'
        header_time=$(cat "../$1" | sed -n '\| src/'"${file:2}$"'| { N; s|.*\n||p; }' | sed -n 's/.*\( [0-9\.]* \).*/\1/p' )
        #echo "header time:" "$header_time"
        sed -i 's/^.*$/& '"$header_time"'/' "${file:2}"
    else
        rm "${file:2}"
    fi
done
cd ..
echo "Summing results..."
find headers-annotated/ -name "*.cpp" -type f -exec cat {} + | sort -s -g -k 1,1 | awk '{
    arr[$1]+=$2
   }
   END {
     for (key in arr) printf("%s\t%s\n", arr[key], key )
   }' \
   | sort -k1 --numeric --reverse > "header_time_rank.log"
echo "wrote to header_time_rank.log"
echo "Finished."
less header_time_rank.log
