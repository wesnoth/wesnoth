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
OPT="$6"
WML_TESTS="$7"
WML_TEST_TIME="$8"
PLAY_TEST="$9"
MP_TEST="${10}"
BOOST_TEST="${11}"
LTO="${12}"
SAN="${13}"

if [ "$OPT" == "-O0" ]; then
    STRICT="true"
    build_timeout=35
else
    STRICT="false"
    build_timeout=40
fi

if [ "$CXXSTD" == "17" ]; then
    STRICT="true"
fi

echo "Using configuration:"
echo "NLS: $NLS"
echo "TOOL: $TOOL"
echo "CC: $CC"
echo "CXX: $CXX"
echo "CXXSTD: $CXXSTD"
echo "OPT: $OPT"
echo "WML_TESTS: $WML_TESTS"
echo "WML_TEST_TIME: $WML_TEST_TIME"
echo "PLAY_TEST: $PLAY_TEST"
echo "MP_TEST: $MP_TEST"
echo "BOOST_TEST: $BOOST_TEST"
echo "LTO: $LTO"
echo "SAN: $SAN"

echo "STRICT: $STRICT"
echo "build_timeout(mins): $build_timeout"

$CXX --version

if [ "$NLS" == "true" ]; then
    if [ "$TOOL" == "cmake" ]; then
        cmake -DENABLE_NLS=true -DENABLE_GAME=false -DENABLE_SERVER=false -DENABLE_CAMPAIGN_SERVER=false -DENABLE_TESTS=false && make VERBOSE=1 -j2
    else
        scons translations build=release --debug=time nls=true jobs=2
    fi
else
    build_start=$(date +%s)

    if [ "$TOOL" == "cmake" ]; then
        echo "max_size = 200M" > $HOME/.ccache/ccache.conf
        echo "compiler_check = content" >> $HOME/.ccache/ccache.conf

        cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_GAME=true -DENABLE_SERVER=true -DENABLE_CAMPAIGN_SERVER=true -DENABLE_TESTS=true -DENABLE_NLS=false \
              -DEXTRA_FLAGS_CONFIG="-pipe" -DOPT="$OPT" -DENABLE_STRICT_COMPILATION="$STRICT" -DENABLE_LTO="$LTO" -DLTO_JOBS=2 -DSANITIZE="$SAN" \
              -DCXX_STD="$CXXSTD" -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache && \
              make VERBOSE=1 -j2
        BUILD_RET=$?

        ccache -s
        ccache -z
    else
        scons wesnoth wesnothd campaignd boost_unit_tests build=release \
              ctool=$CC cxxtool=$CXX cxx_std=$CXXSTD \
              extra_flags_config="-pipe" opt="$OPT" strict="$STRICT" \
              nls=false enable_lto="$LTO" sanitize="$SAN" jobs=2 --debug=time
        BUILD_RET=$?
    fi

    if [ $BUILD_RET != 0 ]; then
        exit $BUILD_RET
    fi

     build_end=$(date +%s)
     if (( build_end-build_start > 60*build_timeout )); then
         echo "Insufficient time remaining to execute unit tests. Exiting now to allow caching to occur. Please restart the job."
         exit 1
     fi

# needed since docker returns the exit code of the final command executed, so a failure needs to be returned if any unit tests fail
    EXIT_VAL=0

    if [ "$WML_TESTS" == "true" ]; then
        echo "Executing run_wml_tests"

        ./run_wml_tests -g -v -c -t "$WML_TEST_TIME"
        RET=$?

        if [ $RET != 0 ]; then
            echo "WML tests failed!"
            EXIT_VAL=$RET
        fi
    fi

    if [ "$PLAY_TEST" == "true" ]; then
        echo "Executing play_test_executor.sh"

        ./utils/travis/play_test_executor.sh
        RET=$?

        if [ $RET != 0 ]; then
            echo "Play tests failed!"
            EXIT_VAL=$RET
        fi
    fi

    if [ "$MP_TEST" == "true" ]; then
        echo "Executing mp_test_executor.sh"

        ./utils/travis/mp_test_executor.sh
        RET=$?

        if [ $RET != 0 ]; then
            echo "MP tests failed!"
            EXIT_VAL=$RET
        fi
    fi

    if [ "$BOOST_TEST" == "true" ]; then
        echo "Executing boost unit tests"

        ./utils/travis/test_executor.sh
        RET=$?

        if [ $RET != 0 ]; then
            echo "Boost tests failed!"
            EXIT_VAL=$RET
        fi
    fi

    if [ -f "errors.log" ]; then
        echo -e "\n*** \n*\n* Errors reported in wml unit tests, here is errors.log...\n*\n*** \n"
        cat errors.log
    fi

    exit $EXIT_VAL
fi
