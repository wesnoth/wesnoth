#!/bin/bash

echo "Using docker:"
echo "BRANCH: $BRANCH"
echo "IMAGE: $IMAGE"
echo "NLS: $NLS"
echo "TOOL: $TOOL"
echo "CC: $CC"
echo "CXX: $CXX"
echo "CXX_STD: $CXX_STD"
echo "CFG: $CFG"
echo "LTO: $LTO"
echo "CACHE_DIR: $CACHE_DIR"

# set the fake display for unit tests
export DISPLAY=:99.0
/sbin/start-stop-daemon --start --quiet --pidfile /tmp/custom_xvfb_99.pid --make-pidfile --background --exec /usr/bin/Xvfb -- :99 -ac -screen 0 1024x768x24

error() { printf '%s\n' "$*"; }
die() { error "$*"; exit 1; }

# print given message ($1) and execute given command; sets EXIT_VAL on failure
execute() {
    local message=$1; shift
    printf 'Executing %s\n' "$message"
    if "$@"; then
        : # success
    else
        EXIT_VAL=$?
        error "$message failed! ($*)"
    fi
}

EXIT_VAL=-1

if [ "$TOOL" == "cmake" ]; then
    export CCACHE_MAXSIZE=3000M
    export CCACHE_COMPILERCHECK=content
    export CCACHE_DIR="$CACHE_DIR"

    cmake -DCMAKE_BUILD_TYPE="$CFG" -DENABLE_GAME=true -DENABLE_SERVER=true -DENABLE_CAMPAIGN_SERVER=true -DENABLE_TESTS=true -DENABLE_NLS="$NLS" \
          -DEXTRA_FLAGS_CONFIG="-pipe" -DENABLE_STRICT_COMPILATION=false -DENABLE_LTO="$LTO" -DLTO_JOBS=2 -DENABLE_MYSQL=false \
          -DCXX_STD="$CXX_STD" -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache && \
          make VERBOSE=1 -j2
    EXIT_VAL=$?

    ccache -s
    ccache -z
else
    scons wesnoth wesnothd campaignd boost_unit_tests build="$CFG" \
        ctool="$CC" cxxtool="$CXX" cxx_std="$CXX_STD" \
        extra_flags_config="-pipe" strict=false forum_user_handler=false \
        nls="$NLS" enable_lto="$LTO" jobs=2 --debug=time
    EXIT_VAL=$?
fi

if [ $EXIT_VAL != 0 ]; then
    exit $EXIT_VAL
fi

# rename debug executables to what the tests expect
if [ "$CFG" == "debug" ]; then
    mv wesnoth-debug wesnoth
    mv wesnothd-debug wesnothd
    mv campaignd-debug campaignd
    mv boost_unit_tests-debug boost_unit_tests
fi

execute "WML tests" ./run_wml_tests -g -v -c -t 20
execute "Play tests" ./utils/travis/play_test_executor.sh
execute "Boost unit tests" ./utils/travis/test_executor.sh

if [ -f "errors.log" ]; then
    error $'\n*** \n*\n* Errors reported in wml unit tests, here is errors.log...\n*\n*** \n'
    cat errors.log
fi

mv wesnoth "$CACHE_DIR"/wesnoth
mv wesnothd "$CACHE_DIR"/wesnothd

exit $EXIT_VAL
