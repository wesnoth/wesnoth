#!/bin/bash

usage()
{
  echo "Usage:" $0 "[OPTIONS]"
  echo
  echo "Computes all the (deep) header dependencies for each file (compilation unit) in"
  echo "the wesnoth project."
  echo
  echo "The calculated dependency lists are placed out of tree, in a subdirectory"
  echo "'headers' of the root of the repostory, in order that they may be conveniently"
  echo "grepped or similar."
  echo
  echo "A ranking of most commonly used headers is generated, in header_rank.log, based"
  echo "on the number of compilation units which use the header."
  echo
  echo "The tool expects the current working directory to be the root directory of the"
  echo "repository."
  echo
  echo -e "Options:"
  echo -e "\t-h\tShows this help."
  echo -e "\t-s\tShow source dependencies."
  echo -e "\t-b\tShow boost dependencies."
  echo -e "\t-i\tShow all /usr/include dependencies."
  echo -e "\t-y\tShow all /usr/bin (system) dependencies."
  echo
  echo -e "\tBy default *all* dependencies are shown."
  echo -e "\tIf multiple flags are passed, the OR of these is shown."
  echo
  echo -e "\t-m arg\tUse a custom pattern. Pass a regexp as an argument to"
  echo -e "\t\tmatch against the paths of included files."
  echo -e "\t\tCan't use this with other options."
  echo
  echo
  echo "Example Usage:"
  echo
  echo -e "\t./build_headers.sh -s"
  echo
  exit 1;
}

echo "Reading options..."
dir_pattern=""
src_pattern="\(src\/\)"
boost_pattern="\(\/usr\/include\/boost\/\)"
incl_pattern="\(\/usr\/include\/\)"
bin_pattern="\(\/usr\/bin\/\)"

while getopts ":hsbiym:" Option
do
  case $Option in
    h )
      usage
      exit 0;
      ;;
    s )
      echo "Adding source includes..."
      if [ -n "$dir_pattern" ]; then
          dir_pattern+="\|"
      fi
      dir_pattern+="$src_pattern"
      ;;
    b )
      echo "Adding boost includes..."
      if [ -n "$dir_pattern" ]; then
          dir_pattern+="\|"
      fi
      dir_pattern+="$boost_pattern"
      ;;
    i )
      echo "Adding /usr/include includes..."
      if [ -n "$dir_pattern" ]; then
          dir_pattern+="\|"
      fi
      dir_pattern+="$incl_pattern"
      ;;
    y )
      echo "Adding bin includes..."
      if [ -n "$dir_pattern" ]; then
          dir_pattern+="\|"
      fi
      dir_pattern+="$bin_pattern"
      ;;
    m )
      echo "Matching against pattern:"
      dir_pattern="$OPTARG"
      echo "$dir_pattern"
      ;;
  esac
done
shift $(($OPTIND - 1))

echo "Final pattern:" "$dir_pattern"

INCLUDE_STR="-Isrc -I/usr/include/SDL -I/usr/include -I/usr/include/pango-1.0 -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/include/fribidi"

echo "Building header include database in wesnoth/headers/..."
[ -d headers ] || mkdir headers
pwd
#find src/ -type f -print0 | xargs -0 ./build_header.sh
cd src
for file in `find . -name "*.cpp" -type f -print0 | xargs -0`; do
    if [ ! -f ../headers/"$file" ]; then
        mkdir -p ../headers/"$file"
        rmdir ../headers/"$file"
    fi
    cd ..
    echo "src/${file:2}"
    #read -p "asdf"
    clang++ -H $INCLUDE_STR "src/${file:2}" 2>&1 >/dev/null | sed -n '/^\.*\. / p' | sed -e 's/^\.* //g' -e ':loop' -e 's|/[[:alnum:]_-\.]*/\.\./|/|g' -e 't loop' | sed -n '/^'"$dir_pattern"'/ p' | sort | uniq >headers/"${file:2}"
    cd src
done
cd ..
echo "ranking headers"
find headers/ -type f -exec cat {} + | sort | uniq -c | sort -k1 --numeric --reverse > "header_rank.log"
echo "wrote to header_rank.log"
echo "Finished."
