#!/bin/bash

# set the fake display for unit tests
export DISPLAY=:99.0
/sbin/start-stop-daemon --start --quiet --pidfile /tmp/custom_xvfb_99.pid --make-pidfile --background --exec /usr/bin/Xvfb -- :99 -ac -screen 0 1024x768x24

# name the parameters
NLS="$1"
TOOL="$2"
CC="$3"
CXX="$4"
CXXSTD="$5"
EXTRA_FLAGS_RELEASE="$6"
WML_TESTS="$7"
WML_TEST_TIME="$8"
PLAY_TEST="$9"
MP_TEST="${10}"
BOOST_TEST="${11}"

# only enable strict builds when no optimizations are done
if [ "$EXTRA_FLAGS_RELEASE" == "-O0" ]; then
  STRICT="true"
else
  STRICT="false"
fi

echo "Using configuration:"
echo "NLS: $NLS"
echo "TOOL: $TOOL"
echo "CC: $CC"
echo "CXX: $CXX"
echo "CXXSTD: $CXXSTD"
echo "EXTRA_FLAGS_RELEASE: $EXTRA_FLAGS_RELEASE"
echo "WML_TESTS: $WML_TESTS"
echo "WML_TEST_TIME: $WML_TEST_TIME"
echo "PLAY_TEST: $PLAY_TEST"
echo "MP_TEST: $MP_TEST"
echo "BOOST_TEST: $BOOST_TEST"

$CXX --version

# if doing the translations, don't build anything else
if [ "$NLS" == "true" ]; then
  if [ "$TOOL" == "cmake" ]; then
    cmake -DENABLE_NLS=true -DENABLE_GAME=false -DENABLE_SERVER=false -DENABLE_CAMPAIGN_SERVER=false -DENABLE_TESTS=false && make VERBOSE=1 -j2
  else
    scons translations build=release --debug=time nls=true jobs=2
  fi
else
# if not doing the translations, build wesnoth, wesnothd, campaignd, boost_unit_tests
  if [ "$TOOL" == "cmake" ]; then
# softlink to build/ for cmake, since that's where the docker mount point is
    cd src/
    ln -s ../build CMakeFiles
    cd ..
    
# run cmake separately so config.h will be seen by the md5 script
    cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_GAME=true -DENABLE_SERVER=true -DENABLE_CAMPAIGN_SERVER=true -DENABLE_TESTS=true -DENABLE_NLS=false -DEXTRA_FLAGS_CONFIG="-pipe" -DEXTRA_FLAGS_RELEASE="$EXTRA_FLAGS_RELEASE" -DENABLE_STRICT_COMPILATION="$STRICT"
# run manual md5 file tracking/mtime modification script for cmake
    python3 cmake_mtime_crc.py
    PY_RES=$?
    if [ "$PY_RES" != "0" ]; then
      rm build/cmake_mtime_crc.txt
      echo "Failed to update cmake file mtimes!"
      exit 1
    fi
    
    make VERBOSE=1 -j2
    
  else
    scons wesnoth wesnothd campaignd boost_unit_tests build=release ctool=$CC cxxtool=$CXX --debug=time extra_flags_config="-pipe" extra_flags_release="$EXTRA_FLAGS_RELEASE" strict="$STRICT" cxx_std=$CXXSTD nls=false jobs=2
  fi
  
# check if the build failed
  BUILD_RET=$?
  if [ $BUILD_RET != 0 ]; then
# if the failed build was using cmake, clear out everything so it's recompiled from scratch next time
# otherwise, it is possible for there to be source files that didn't change between runs, but still need to be recompiled, and they would be missed which would corrupt the cache
# this is especially possible when there are unrelated commits that don't involve C++ changes made between the original bad commit and the commit fixing the issue
    if [ "$TOOL" = "cmake" ]; then
      rm -R build/*
    fi
    
    exit $BUILD_RET
  fi
  
# needed since docker returns the exit code of the final comman executed, so a failure needs to be returned if any unit tests fail
  EXIT_VAL=0
  
  if [ "$WML_TESTS" == "true" ]; then
    echo "Executing run_wml_tests"
    ./run_wml_tests -g -v -c -t "$WML_TEST_TIME"
    RET=$?
    if [ $RET != 0 ]; then
      echo "WML Tests Failed!"
      EXIT_VAL=$RET
    fi
  fi
  
  if [ "$PLAY_TEST" == "true" ]; then
    echo "Executing play_test_executor.sh"
    ./utils/travis/play_test_executor.sh
    RET=$?
    if [ $RET != 0 ]; then
      echo "Play Tests Failed!"
      EXIT_VAL=$RET
    fi
  fi
  
  if [ "$MP_TEST" == "true" ]; then
    echo "Executing mp_test_executor.sh"
    ./utils/travis/mp_test_executor.sh
    RET=$?
    if [ $RET != 0 ]; then
      echo "MP Tests Failed!"
      EXIT_VAL=$RET
    fi
  fi
  
  if [ "$BOOST_TEST" == "true" ]; then
    echo "Executing boost unit tests"
    ./utils/travis/test_executor.sh
    RET=$?
    if [ $RET != 0 ]; then
      echo "Boost Tests Failed!"
      EXIT_VAL=$RET
    fi
  fi
  
  if [ -f "errors.log" ]; then
    echo -e "\n*** \n*\n* Errors reported in wml unit tests, here is errors.log...\n*\n*** \n"
    cat errors.log
  fi
  
  exit $EXIT_VAL
fi
